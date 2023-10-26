//
// Created by lada on 10/17/23.
//

#ifndef EARTH_VISUALIZATION_TILE_H
#define EARTH_VISUALIZATION_TILE_H

#include <vector>
#include <cmath>
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp> // glm::mat4
#include "TileResources.h"
#include "../ellipsoid.h"

class CameraParams {

};

class Tile {
private:
    std::vector<std::shared_ptr<TileResources>> lodResources;
    double latitude, longitude, latitudeWidth, longitudeWidth;
    glm::vec3 geocentricPosition;
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
            double screenSpaceWidth, double distanceToCamera, double cameraViewAngle) {
        // Determine the appropriate level of detail (LOD) based on the screen-space error.

        int level;
        for (level = lodResources.size() - 1; level >= 0; level--) {
            auto &lod = lodResources[level];
            // Define the geometric error simply as the inverse of the number of triangles in a tile.
            auto geometricError = 1.0 / lod->getMesh().size();
            double screenSpaceError = computeScreenSpaceError(screenSpaceWidth, distanceToCamera,
                                                              cameraViewAngle, geometricError);
            if (screenSpaceError < 5.0) {
                break;
            }
            // std::cout << "[" << level << "] screen space error: " << screenSpaceError << std::endl;
        }
        level = std::min(level, static_cast<int>(lodResources.size() - 1));
        return lodResources[level];
    }

    std::shared_ptr<TileResources> getResourcesByLevel(int level) {
        assert(level < lodResources.size());
        return lodResources[level];
    }

    void addResources(const std::shared_ptr<TileResources> &resources, int level) {
        // Assumes resources are added from coarse to fine for simplicity.
        // Check it is true.
        assert(level > lastLevel);
        lastLevel = level;

        // Cross-reference resources of neighboring LODs
        if (!lodResources.empty()) {
            auto lastResources = lodResources.back();
            lastResources->finerResources.push_back(resources);
            resources->coarserResources = lastResources;
        }
        // Add resources to the current tile
        lodResources.push_back(resources);
    }

    /**
     * From: Geometric Approach - Testing Points and Spheres
     * https://cgvr.informatik.uni-bremen.de/teaching/cg_literatur/lighthouse3d_view_frustum_culling/index.html
     *
     * @param viewProjectionMatrix
     * @return
     */
    [[nodiscard]] bool isInViewFrustum(const glm::mat4 &viewProjectionMatrix) const {
        // Calculate the bounding sphere of the tile based on its geocentric position and size.
        // You can adapt this based on how you define your bounding volume.
        // For a sphere, you'd calculate its center and radius.
        glm::vec3 tileCenter = getGeocentricPosition();
        double tileRadius = getTileRadius();

        // Extract the frustum planes from the viewProjectionMatrix.
        // https://www8.cs.umu.se/kurser/5DV051/HT12/lab/plane_extraction.pdf
        glm::vec4 planes[6];
        for (int i = 0; i < 4; i++) {
            planes[0][i] = viewProjectionMatrix[i][3] + viewProjectionMatrix[i][0];  // Left
            planes[1][i] = viewProjectionMatrix[i][3] - viewProjectionMatrix[i][0];  // Right
            planes[2][i] = viewProjectionMatrix[i][3] + viewProjectionMatrix[i][1];  // Bottom
            planes[3][i] = viewProjectionMatrix[i][3] - viewProjectionMatrix[i][1];  // Top
            planes[4][i] = viewProjectionMatrix[i][3] + viewProjectionMatrix[i][2];  // Near
            planes[5][i] = viewProjectionMatrix[i][3] - viewProjectionMatrix[i][2];  // Far
        }
        // Check if the tile's bounding sphere is inside all frustum planes.
        for (auto plane: planes) {
            float signedDistance = glm::dot(plane, glm::vec4(tileCenter, 1.0));

            // If the signed distance is less than the negative radius, the sphere is completely outside the frustum.
            if (signedDistance < -tileRadius) {
                return false;
            }
        }

        // If the tile's bounding sphere is not completely outside any frustum plane, it's in the view frustum.
        return true;
    }

    /**
     * Uses longitude and latitude to project the centre of the tile
     * onto the surface of the ellipsoid.
     */
    void updateGeocentricPosition(Ellipsoid &ellipsoid) {
        auto toRadsCoeff = static_cast<float>(M_PI / 180.0);
        double longitudeCentre = longitude + longitudeWidth / 2.0;
        double latitudeCentre = latitude + latitudeWidth / 2.0;

        auto upperLeftCorner = glm::vec3(longitude, latitude, 0) * toRadsCoeff;
        auto upperRightCorner = glm::vec3(longitude + longitudeWidth, latitude, 0) * toRadsCoeff;
        auto geocentricUpperLeftCorner = ellipsoid.convertGeodeticToGeocentric(upperLeftCorner);
        auto geocentricUpperRightCorner = ellipsoid.convertGeodeticToGeocentric(upperRightCorner);
        tileWidth = glm::length(geocentricUpperRightCorner - geocentricUpperLeftCorner);

        auto tileCentre = glm::vec3(longitudeCentre, latitudeCentre, 0) * toRadsCoeff;
        geocentricPosition = ellipsoid.convertGeodeticToGeocentric(tileCentre);
    }

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
