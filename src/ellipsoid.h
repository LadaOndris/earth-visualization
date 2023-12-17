
#ifndef EARTH_VISUALIZATION_ELLIPSOID_H
#define EARTH_VISUALIZATION_ELLIPSOID_H

#include <glm/vec3.hpp>
#include <glm/geometric.hpp>
#include <vector>

extern glm::vec3 REAL_RADII_METERS;

/**
 * The implementation of the Ellipsoid class is based on the implementation
 * provided in the book:
 * "P. Cozzi, K. Ring, 3D Engine Design for Virtual Globes. A. K. Peters, Ltd., 2011."
 */
class Ellipsoid {
public:
    Ellipsoid(double x, double y, double z);

    explicit Ellipsoid(glm::vec3 radii);

    glm::vec3 getRadii() const;

    glm::vec3 getRadiiSquared() const;

    glm::vec3 getOneOverRadiiSquared() const;

    /**
     * Computes the geodetic surface normal given a point on the surface of the ellipsoid.
     * @param vec Point on the surface of the ellipsoid.
     * @return
     */
    glm::vec3 convertGeocentricToGeocentricSurfaceNormal(glm::vec3 point) const;

    /**
     * Converts a geocentric poitn (X, Y, Z) to a point geodetic point (longitude, latitude, 0).
     */
    glm::vec3 convertGeocentricToGeodetic(glm::vec3 point) const;

    std::vector<glm::vec3> projectGeodeticCoordsOntoSurface(const std::vector<glm::vec3> &geodeticCoords) const;

    glm::vec3 projectGeocentricPointOntoSurface(glm::vec3 geocentricPoint) const;

    glm::vec3 convertGeographicToGeodeticSurfaceNormal(glm::vec3 geographic) const;

    glm::vec3 convertGeodeticToGeocentric(glm::vec3 geodetic) const;

    bool isPointOnTheOutside(glm::vec3);

    glm::vec3 getGeocentricPosition() const;

    float getRealityScaleFactor() const;

    static Ellipsoid &wgs84() {
        // Equatorial radius (m), Equatorial radius (m), Polar radius (m)
        auto radii = REAL_RADII_METERS;
        static Ellipsoid ellipsoid(radii);
        return ellipsoid;
    }

    static Ellipsoid &unitSphere() {
        static Ellipsoid ellipsoid(1.0, 1.0, 1.0);
        return ellipsoid;
    }

    static Ellipsoid &unitSphereWithCorrectRatio() {
        auto radii = glm::normalize(REAL_RADII_METERS);
        static Ellipsoid ellipsoid(radii);
        return ellipsoid;
    }

    [[nodiscard]] bool isPointFacingCamera(glm::vec3 cameraPosition, glm::vec3 geocentricPosition) const;

private:
    glm::vec3 radii;
    glm::vec3 radiiSquared;
    glm::vec3 oneOverRadiiSquared;
};


#endif //EARTH_VISUALIZATION_ELLIPSOID_H
