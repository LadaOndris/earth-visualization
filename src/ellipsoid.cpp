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

/**
 * Computes the geodetic surface normal given a point on the surface of the ellipsoid.
 * @param vec Point on the surface of the ellipsoid.
 * @return
 */
glm::vec3 Ellipsoid::geodeticSurfaceNormalFromWGS84(glm::vec3 point) {
    auto normal = point * oneOverRadiiSquared;
    return glm::normalize(normal);
}

glm::vec3 Ellipsoid::geodeticSurfaceNormalFromGeodetic(glm::vec3 geodetic) {
    double longitude = geodetic.x;
    double latitude = geodetic.y;

    double cosLatitude = cos(latitude);
    auto normal = glm::vec3(cosLatitude * cos(longitude),
                            cosLatitude * sin(longitude),
                            sin(latitude));
    return normal;
}

glm::vec3 Ellipsoid::convertGeographicToWGS84(glm::vec3 geodetic) {
    double longitude = geodetic.x;
    double latitude = geodetic.y;
    float height = geodetic.z;

    auto n = geodeticSurfaceNormalFromGeodetic(geodetic);
    auto k = radiiSquared * n;
    float gamma = std::sqrt(k.x * n.x + k.y * n.y + k.z * n.z);
    // Point on the surface determined as determined by the normal.
    auto rSurface = k / gamma;

    return rSurface + (n * height);
}

glm::vec3 Ellipsoid::convertWGS84ToGeographic(glm::vec3 point) {
    auto normal = geodeticSurfaceNormalFromWGS84(point);
    auto geodetic = glm::vec3(std::atan2(normal.y, normal.x),
                              std::asin(normal.z / glm::length(normal)),
                              0);
    return geodetic;
}

std::vector<glm::vec3> Ellipsoid::projectPointsOntoSurface(std::vector<glm::vec3> points) {
    // Project points from unit sphere to the custom ellipsoid
    std::vector<glm::vec3> projectedPoints;
    for (const auto& point : points) {
        // Step 2: Convert from unit sphere to geodetic coordinates on the unit sphere
        glm::vec3 geodeticOnUnitSphere = convertWGS84ToGeographic(point);

        // Step 3: Convert from geodetic coordinates on unit sphere to WGS84 coordinates on the custom ellipsoid
        glm::vec3 projectedPoint = convertGeographicToWGS84(geodeticOnUnitSphere);

        // Add the projected point to the result
        projectedPoints.push_back(projectedPoint);
    }
    return projectedPoints;
}



