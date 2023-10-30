
#ifndef EARTH_VISUALIZATION_ELLIPSOID_H
#define EARTH_VISUALIZATION_ELLIPSOID_H

#include <glm/vec3.hpp>
#include <vector>

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

    static Ellipsoid &wgs84() {
        // Equatorial radius (m), Equatorial radius (m), Polar radius (m)
        static Ellipsoid ellipsoid(6378137.0, 6378137.0, 6356752.314245);
        return ellipsoid;
    }

    static Ellipsoid &unitSphere() {
        static Ellipsoid ellipsoid(1.0, 1.0, 1.0);
        return ellipsoid;
    }

private:
    glm::vec3 radii;
    glm::vec3 radiiSquared;
    glm::vec3 oneOverRadiiSquared;
};


#endif //EARTH_VISUALIZATION_ELLIPSOID_H
