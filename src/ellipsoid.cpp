//
// Created by lada on 9/24/23.
//

#include <glm/geometric.hpp>
#include "ellipsoid.h"

Ellipsoid::Ellipsoid(glm::vec3 radii) : radii(radii) {
    radiiSquared = glm::vec3(radii.x * radii.x,
                             radii.y * radii.y,
                             radii.z * radii.z);
    oneOverRadiiSquared = 1.0f / radiiSquared;
}

Ellipsoid::Ellipsoid(double x, double y, double z) : Ellipsoid(glm::vec3(x, y, z)) {

}

glm::vec3 Ellipsoid::getRadii() const {
    return radii;
}

glm::vec3 Ellipsoid::getOneOverRadiiSquared() const {
    return oneOverRadiiSquared;
}

std::vector<glm::vec3> Ellipsoid::projectGeodeticCoordsOntoSurface(const std::vector<glm::vec3> &geodeticCoords) const {
    // Project geodeticCoords from unit sphere to the custom ellipsoid
    std::vector<glm::vec3> projectedPoints;
    for (const auto &point: geodeticCoords) {
        auto projectedPoint = convertGeodeticToGeocentric(point);
        // Add the projected point to the result
        projectedPoints.push_back(projectedPoint);
    }
    return projectedPoints;
}


glm::vec3 Ellipsoid::convertGeographicToGeodeticSurfaceNormal(glm::vec3 geographic) const {
    float longitude = geographic.x;
    float latitude = geographic.y;

    float cosLatitude = std::cos(latitude);
    glm::vec3 normal = glm::vec3(
            cosLatitude * std::cos(longitude),
            cosLatitude * std::sin(longitude),
            std::sin(latitude));

    return normal;
}

glm::vec3 Ellipsoid::convertGeodeticToGeocentric(glm::vec3 geodetic) const {
    float height = geodetic.z;

    glm::vec3 n = convertGeographicToGeodeticSurfaceNormal(geodetic);
    glm::vec3 k = radiiSquared * n;
    float gamma = std::sqrt(k.x * n.x + k.y * n.y + k.z * n.z);
    // Point on the surface determined as determined by the normal.
    glm::vec3 rSurface = k / gamma;

    return rSurface + (n * height);
}


glm::vec3 Ellipsoid::projectGeocentricPointOntoSurface(glm::vec3 geocentricPoint) const {
    // Convert from unit sphere to geodetic coordinates on the unit sphere
    glm::vec3 geodeticOnUnitSphere = convertGeocentricToGeodetic(geocentricPoint);
    // Convert from geodetic coordinates on unit sphere to WGS84 coordinates on the custom ellipsoid
    glm::vec3 projectedPoint = convertGeodeticToGeocentric(geodeticOnUnitSphere);
    return projectedPoint;
}


glm::vec3 Ellipsoid::convertGeocentricToGeodetic(glm::vec3 point) const {
    auto normal = convertGeocentricToGeocentricSurfaceNormal(point);
    auto geodetic = glm::vec3(std::atan2(normal.y, normal.x),
                              std::asin(normal.z / glm::length(normal)),
                              0);
    return geodetic;
}

glm::vec3 Ellipsoid::convertGeocentricToGeocentricSurfaceNormal(glm::vec3 point) const {
    auto normal = point * oneOverRadiiSquared;
    return glm::normalize(normal);
}


bool Ellipsoid::isPointOnTheOutside(glm::vec3 point) {
    auto pointSquared = point * point;
    auto components = pointSquared * oneOverRadiiSquared;
    float result = components.x + components.y + components.z;
    return result > 1;
}



