//
// Created by lada on 10/17/23.
//

#include "Tile.h"
#include "TileResources.h"
#include "../utils.h"


std::shared_ptr<TileResources> Tile::getResources(
        double screenSpaceWidth, double distanceToCamera, const Camera &camera) {
    static double maxAngle = 0;

//    double viewingAngle = getViewingAngle(camera);
//    double viewingAngleNormalized = std::fabs(viewingAngle / 3.14159265 * 2);
//    viewingAngleNormalized = std::min(viewingAngleNormalized / 0.4, 1.);
//    double viewingCoeff = std::pow(1 - viewingAngleNormalized, 2); // -std::log(viewingAngleNormalized + 0.001);
//    maxAngle = std::max(maxAngle, viewingAngleNormalized);
//    std::cout << maxAngle << std::endl;

    // Determine the appropriate level of detail (LOD) based on the screen-space error.
    int level;
    for (level = lodResources.size() - 1; level >= 0; level--) {
        auto &lod = lodResources[level];
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
    level = std::min(level, static_cast<int>(lodResources.size() - 1));
    //return lodResources[lodResources.size() - 1];
    return lodResources[level];
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
    assert(level < lodResources.size());
    return lodResources[level];
}

bool Tile::isTileWithinTexture(const std::shared_ptr<Texture> &texture) const {
    auto textureOffset = texture->getGeodeticOffset();
    auto textureLongWidth = texture->getLongitudeWidth();
    auto textureLatWidth = texture->getLatitudeWidth();

    if (this->longitude < textureOffset[0] ||
        this->latitude < textureOffset[1]) {
        return false;
    }
    if (this->longitude > textureOffset[0] + textureLongWidth ||
        this->latitude > textureOffset[1] + textureLatWidth) {
        return false;
    }

    return true;
}

void Tile::addResources(const std::shared_ptr<TileResources> &resources, int level) {
    // Assumes resources are added from coarse to fine for simplicity.
    // Check it is true.
    assert(level > lastLevel);
    if (!isTileWithinTexture(resources->getTexture(TextureType::Day))) {
        throw std::runtime_error("The tile is located outside of the resources definition.");
    }

    lastLevel = level;

    // Cross-reference resources of neighboring LODs
    if (!lodResources.empty()) {
        auto lastResources = lodResources.back();
        assert(lastResources->getMesh().size() > resources->getMesh().size());
        resources->finerResources.push_back(lastResources);
        lastResources->coarserResources = resources;
    }
    // Add resources to the current tile
    lodResources.push_back(resources);
}


[[nodiscard]] bool Tile::isInViewFrustum(const Frustum &frustum) const {

    unsigned int cornersOutsideFrustum = 0;
    auto tileCorners = getGeocentricTileCorners();

    for (int cornerIndex = 0; cornerIndex < tileCorners.size(); cornerIndex++) {
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
    float dotProduct = glm::dot(normal, toCamera);
    // If the dot product is positive, the tile is facing the camera.
    return dotProduct > 0.0;
}

/**
 * Uses longitude and latitude to project the centre of the tile
 * onto the surface of the ellipsoid.
 */
void Tile::updateGeocentricPosition(Ellipsoid &ellipsoid) {
    double longitudeCentre = longitude + longitudeWidth / 2.0;
    double latitudeCentre = latitude + latitudeWidth / 2.0;

    auto upperLeftCorner = utils::convertToRads(glm::vec3(longitude, latitude, 0));
    auto upperRightCorner = utils::convertToRads(glm::vec3(longitude + longitudeWidth, latitude, 0));
    auto lowerLeftCorner = utils::convertToRads(glm::vec3(longitude, latitude + latitudeWidth, 0));
    auto lowerRightCorner = utils::convertToRads(glm::vec3(longitude + longitudeWidth, latitude + latitudeWidth, 0));

    auto geocentricUpperLeftCorner = ellipsoid.convertGeodeticToGeocentric(upperLeftCorner);
    auto geocentricUpperRightCorner = ellipsoid.convertGeodeticToGeocentric(upperRightCorner);
    auto geocentricLowerLeftCorner = ellipsoid.convertGeodeticToGeocentric(lowerLeftCorner);
    auto geocentricLowerRightCorner = ellipsoid.convertGeodeticToGeocentric(lowerRightCorner);
    corners = std::array<glm::vec3, 4>({
                                               geocentricUpperLeftCorner, geocentricUpperRightCorner,
                                               geocentricLowerLeftCorner, geocentricLowerRightCorner
                                       });
    tileWidth = glm::length(geocentricUpperRightCorner - geocentricUpperLeftCorner);

    auto tileCentre = utils::convertToRads(glm::vec3(longitudeCentre, latitudeCentre, 0));
    geocentricPosition = ellipsoid.convertGeodeticToGeocentric(tileCentre);

    normal = ellipsoid.convertGeographicToGeodeticSurfaceNormal(tileCentre);
}

[[nodiscard]] std::array<glm::vec3, 4> Tile::getGeocentricTileCorners() const {
    return corners;
}

[[nodiscard]] std::array<std::pair<glm::vec3, glm::vec3>, 4> Tile::getEdges() const {
    return {
            std::pair(corners[0], corners[1]),
            std::pair(corners[1], corners[2]),
            std::pair(corners[2], corners[3]),
            std::pair(corners[3], corners[0])
    };
}