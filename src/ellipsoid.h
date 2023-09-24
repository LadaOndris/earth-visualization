
#ifndef EARTH_VISUALIZATION_ELLIPSOID_H
#define EARTH_VISUALIZATION_ELLIPSOID_H

#include <glm/vec3.hpp>

class Ellipsoid {
public:
    Ellipsoid(double x, double y, double z);

    explicit Ellipsoid(glm::vec3 radii);

    glm::vec3 getRadii() const;

    glm::vec3 geodeticSurfaceNormalFromWGS84(glm::vec3 point);

    /**
     *
     * @param geodetic Tuple (longitude, latitude, height)
     * @return
     */
    glm::vec3 geodeticSurfaceNormalFromGeodetic(glm::vec3 geodetic);

    glm::vec3 convertGeographicToWGS84(glm::vec3 geodetic);

    glm::vec3 convertWGS84ToGeographic(glm::vec3 point);

    static Ellipsoid &wgs84() {
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
