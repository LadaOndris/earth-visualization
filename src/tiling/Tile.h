//
// Created by lada on 10/17/23.
//

#ifndef EARTH_VISUALIZATION_TILE_H
#define EARTH_VISUALIZATION_TILE_H

#include <memory>
#include <vector>
#include <cmath>
#include <stdexcept>
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp> // glm::mat4
#include <glm/geometric.hpp>
#include <array>
#include "../ellipsoid.h"

class TileResources;

class Tile {
private:
    std::vector<std::shared_ptr<TileResources>> lodResources;
    double latitude, longitude, latitudeWidth, longitudeWidth;
    glm::vec3 geocentricPosition;
    std::array<glm::vec3, 4> corners;
    // Normal of the face of the tile.
    glm::vec3 normal;
    double tileWidth;

    int lastLevel = -1;

    /**
     * Compute the screen-space error based on the given parameters.
     *
     * @param screenSpaceWidth The width of the screen-space (x).
     * @param viewFrustrumWidth The width of the view frustum (w).
     * @param distanceToCamera The distance from the center of the tile to the camera position (d).
     * @param cameraViewAngle The view angle of the camera (theta) in radians.
     * @param geometricError The geometric error of the object, given the specific texture (e).
     * @return The calculated screen-space error.
     */
    double computeScreenSpaceError(double screenSpaceWidth, double distanceToCamera,
                                   double cameraViewAngle, double geometricError) {
        double viewFrustrumWidth = 2 * distanceToCamera * std::tan(cameraViewAngle / 2.0);
        double screenSpaceError = screenSpaceWidth * geometricError / viewFrustrumWidth;
        return screenSpaceError;
    }


public:
    explicit Tile(double latitude, double longitude, double latitudeWidth, double longitudeWidth)
            : latitude(latitude), longitude(longitude),
              latitudeWidth(latitudeWidth), longitudeWidth(longitudeWidth),
              geocentricPosition(glm::vec3(0, 0, 0)) {
    }

    /**
     * Selects the tile resources for the appropriate level of detail.
     *
     * We can use these quantities to compute the screen-space error and choose the LOD
     * with the highest allowed screen-space error.
     *
     * As the user zooms in, the screen space error becomes larger for all LODs.
     * Thresholding at a certain value does the trick. We start at the lower resolution
     * and increase the resolution until we get below a certain error.
     *
     * The following parameters play a role in which LOD gets selected:
     * - the width of the screen-space (x)
     * - distance from the center of the tile to the camera position (d)
     * - view angle of the camera (theta)
     */
    std::shared_ptr<TileResources> getResources(
            double screenSpaceWidth, double distanceToCamera, double cameraViewAngle);

    std::shared_ptr<TileResources> getResourcesByLevel(int level);

    /**
     * Check the coords of this tile are within the coords of the resources.
     */
    bool isTileWithinResources(const std::shared_ptr<TileResources> &resources) const;

    void addResources(const std::shared_ptr<TileResources> &resources, int level);

    /**
     * From: Geometric Approach - Testing Points and Spheres
     * https://cgvr.informatik.uni-bremen.de/teaching/cg_literatur/lighthouse3d_view_frustum_culling/index.html
     *
     * @param viewProjectionMatrix
     * @return
     */
    [[nodiscard]] bool isInViewFrustum(const glm::mat4 &viewProjectionMatrix) const;

    [[nodiscard]] unsigned char sumOfBits(unsigned char var) const;

    [[nodiscard]] bool isFacingCamera(const glm::vec3 &cameraPosition) const;

    /**
     * Uses longitude and latitude to project the centre of the tile
     * onto the surface of the ellipsoid.
     */
    void updateGeocentricPosition(Ellipsoid &ellipsoid);

    [[nodiscard]] std::array<glm::vec3, 4> getGeocentricTileCorners() const;

    [[nodiscard]] std::array<std::pair<glm::vec3, glm::vec3>, 4> getEdges() const;


    [[nodiscard]] double getTileRadius() const {
        return tileWidth / 2.0;
    }

    [[nodiscard]] glm::vec3 getGeocentricPosition() const {
        return geocentricPosition;
    }

    [[nodiscard]] double getLatitude() const {
        return latitude;
    }

    [[nodiscard]] double getLongitude() const {
        return longitude;
    }

    [[nodiscard]] double getLatitudeWidth() const {
        return latitudeWidth;
    }

    [[nodiscard]] double getLongitudeWidth() const {
        return longitudeWidth;
    }
};


#endif //EARTH_VISUALIZATION_TILE_H
