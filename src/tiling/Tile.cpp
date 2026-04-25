//
// Created by lada on 10/17/23.
//

#include "Tile.h"
#include "TileResources.h"
#include "../utils.h"


std::shared_ptr<TileResources> Tile::getResources(
        double screenSpaceWidth, double distanceToCamera, const Camera &camera) {
    // static double maxAngle = 0;
//    double viewingAngle = getViewingAngle(camera);
//    double viewingAngleNormalized = std::fabs(viewingAngle / 3.14159265 * 2);
//    viewingAngleNormalized = std::min(viewingAngleNormalized / 0.4, 1.);
//    double viewingCoeff = std::pow(1 - viewingAngleNormalized, 2); // -std::log(viewingAngleNormalized + 0.001);
//    maxAngle = std::max(maxAngle, viewingAngleNormalized);
//    std::cout << maxAngle << std::endl;

    // Determine the appropriate level of detail (LOD) based on the screen-space error.
    int level;
    for (level = _lodResources.size() - 1; level >= 0; level--) {
        auto &lod = _lodResources[level];
        // Define the geometric error simply as the inverse of the number of triangles in a tile.
        // times a coefficient depending on the viewing angle.

        auto geometricError = 1.0 / lod->getMesh().size();

        double screenSpaceError = computeScreenSpaceError(screenSpaceWidth, distanceToCamera,
                                                          camera.getFov(), geometricError);
        if (screenSpaceError < 10.0) {
            break;
        }
        // std::cout << "[" << level << "] screen space error: " << screenSpaceError << std::endl;
    }
    // Avoid out-of-bounds indexing
    level = std::max(level, 0);
    level = std::min(level, static_cast<int>(_lodResources.size() - 1));
    //return _lodResources[_lodResources.size() - 1];
    return _lodResources[level];
}

/**
 * Returns the angle between the camera viewing vector and the position of the the tile.
 * @return Angle in radians.
 */
double Tile::getViewingAngle(const Camera &camera) const {
    glm::vec3 cameraGeocentricPos = camera.getPosition();
    auto tilePos = getGeocentricPosition();
    auto toTile = glm::normalize(tilePos - cameraGeocentricPos);
    auto toTarget = glm::normalize(camera.getTarget() - cameraGeocentricPos);
    double angle = std::acos(glm::dot(toTile, toTarget));
    return angle;
}

std::shared_ptr<TileResources> Tile::getResourcesByLevel(int level) {
    assert(level < static_cast<int>(_lodResources.size()));
    return _lodResources[level];
}

bool Tile::isTileWithinTexture(const std::shared_ptr<Texture> &texture) const {
    auto textureOffset = texture->getGeodeticOffset();
    auto textureLongWidth = texture->getLongitudeWidth();
    auto textureLatWidth = texture->getLatitudeWidth();

    if (this->_longitude < static_cast<double>(textureOffset[0]) ||
        this->_latitude < static_cast<double>(textureOffset[1])) {
        return false;
    }
    if (this->_longitude > static_cast<double>(textureOffset[0]) + textureLongWidth ||
        this->_latitude > static_cast<double>(textureOffset[1]) + textureLatWidth) {
        return false;
    }

    return true;
}

void Tile::addResources(const std::shared_ptr<TileResources> &resources, int level) {
    // Assumes resources are added from coarse to fine for simplicity.
    // Check it is true.
    assert(level > _lastLevel);
    if (!isTileWithinTexture(resources->getTexture(TextureType::Day))) {
        throw std::runtime_error("The tile is located outside of the resources definition.");
    }

    _lastLevel = level;

    // Cross-reference resources of neighboring LODs
    if (!_lodResources.empty()) {
        auto lastResources = _lodResources.back();
        assert(lastResources->getMesh().size() < resources->getMesh().size());
        resources->finerResources.push_back(lastResources);
        lastResources->coarserResources = resources;
    }
    // Add resources to the current tile
    _lodResources.push_back(resources);
}


[[nodiscard]] bool Tile::isInViewFrustum(const Frustum &frustum) const {

    unsigned int cornersOutsideFrustum = 0;
    auto tileCorners = getGeocentricTileCorners();

    for (size_t cornerIndex = 0; cornerIndex < tileCorners.size(); cornerIndex++) {
        auto tileCorner = tileCorners[cornerIndex];

        if (frustum.isPointOutside(tileCorner)) {
            cornersOutsideFrustum |= (1 << cornerIndex);
        }
    }

    if (sumOfBits(cornersOutsideFrustum) < 4) {
        return true;
    }

    // Check for intersection between tile edges and frustum planes
    auto tileEdges = getEdges();
    for (const auto &edge: tileEdges) {
        if (frustum.intersectsEdge(edge)) {
            return true;
        }
    }
    // No corner is inside frustum and
    // no edge of the tile intersets the frustum.
    return false;
}

[[nodiscard]] unsigned char Tile::sumOfBits(unsigned char var) const {
    unsigned int sum = 0;
    while (var > 0) {
        sum += (var & 0x1);
        var >>= 1;
    }
    return sum;
}

[[nodiscard]] bool Tile::isFacingCamera(const glm::vec3 &cameraPosition) const {
    // Calculate the vector from the tile's center to the camera position.
    glm::vec3 toCamera = cameraPosition - getGeocentricPosition();

    // Calculate the dot product between the normal and the vector to the camera.
    float dotProduct = glm::dot(_normal, toCamera);
    // If the dot product is positive, the tile is facing the camera.
    return dotProduct > 0.0f;
}

/**
 * Uses longitude and latitude to project the centre of the tile
 * onto the surface of the ellipsoid.
 */
void Tile::updateGeocentricPosition(Ellipsoid &ellipsoid) {
    double longitudeCentre = _longitude + _longitudeWidth / 2.0;
    double latitudeCentre = _latitude + _latitudeWidth / 2.0;

    auto upperLeftCorner = utils::convertToRads(glm::vec3(_longitude, _latitude, 0));
    auto upperRightCorner = utils::convertToRads(glm::vec3(_longitude + _longitudeWidth, _latitude, 0));
    auto lowerLeftCorner = utils::convertToRads(glm::vec3(_longitude, _latitude + _latitudeWidth, 0));
    auto lowerRightCorner = utils::convertToRads(glm::vec3(_longitude + _longitudeWidth, _latitude + _latitudeWidth, 0));

    auto geocentricUpperLeftCorner = ellipsoid.convertGeodeticToGeocentric(upperLeftCorner);
    auto geocentricUpperRightCorner = ellipsoid.convertGeodeticToGeocentric(upperRightCorner);
    auto geocentricLowerLeftCorner = ellipsoid.convertGeodeticToGeocentric(lowerLeftCorner);
    auto geocentricLowerRightCorner = ellipsoid.convertGeodeticToGeocentric(lowerRightCorner);
    _corners = std::array<glm::vec3, 4>({
                                               geocentricUpperLeftCorner, geocentricUpperRightCorner,
                                               geocentricLowerLeftCorner, geocentricLowerRightCorner
                                       });
    _tileWidth = glm::length(geocentricUpperRightCorner - geocentricUpperLeftCorner);

    auto tileCentre = utils::convertToRads(glm::vec3(longitudeCentre, latitudeCentre, 0));
    _geocentricPosition = ellipsoid.convertGeodeticToGeocentric(tileCentre);

    _normal = ellipsoid.convertGeographicToGeodeticSurfaceNormal(tileCentre);
}

[[nodiscard]] std::array<glm::vec3, 4> Tile::getGeocentricTileCorners() const {
    return _corners;
}

[[nodiscard]] std::array<std::pair<glm::vec3, glm::vec3>, 4> Tile::getEdges() const {
    return {
            std::pair(_corners[0], _corners[1]),
            std::pair(_corners[1], _corners[2]),
            std::pair(_corners[2], _corners[3]),
            std::pair(_corners[3], _corners[0])
    };
}