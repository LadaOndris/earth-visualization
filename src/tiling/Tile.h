//
// Created by lada on 10/17/23.
//

#ifndef EARTH_VISUALIZATION_TILE_H
#define EARTH_VISUALIZATION_TILE_H

#include <vector>
#include <cmath>
#include "TileResources.h"

class CameraParams {

};

class Tile {
private:
    std::vector<std::shared_ptr<TileResources>> lodResources;
    double latitude, longitude, latitudeWidth, longitudeWidth;
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
            : latitude(latitude), longitude(longitude), latitudeWidth(latitudeWidth), longitudeWidth(longitudeWidth) {
    }

    /**
     * Selects the tile resources for the appropriate level of detail.
     *
     * We can use these quantities to compute the screen-space error and choose the LOD
     * with the lowest screen-space error above a certain threshold.
     *
     * As the user zooms in, the screen space error becomes larger for all LODs.
     * Thresholding at a certain value does the trick.
     *
     * The following parameters play a role in which LOD gets selected:
     * - the width of the screen-space (x)
     * - distance from the center of the tile to the camera position (d)
     * - view angle of the camera (theta)
     */
    std::shared_ptr<TileResources> getResources(
            double screenSpaceWidth, double distanceToCamera, double cameraViewAngle) {
        // Determine the appropriate level of detail (LOD) based on the screen-space error.

        int level = 0;
        for (auto lod: lodResources) {
            // Define the geometric error simply as the inverse of the number of triangles in a tile.
            double geometricError = 1.0 / lod->getMesh().size();
            double screenSpaceError = computeScreenSpaceError(screenSpaceWidth, distanceToCamera,
                                                              cameraViewAngle, geometricError);
            if (screenSpaceError > 0.7) {
                break;
            }
            //std::cout << "[" << level << "] screen space error: " << screenSpaceError << std::endl;
            level++;
        }

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

    double getLatitude() const {
        return latitude;
    }

    double getLongitude() const {
        return longitude;
    }

    double getLatitudeWidth() const {
        return latitudeWidth;
    }

    double getLongitudeWidth() const {
        return longitudeWidth;
    }
};


#endif //EARTH_VISUALIZATION_TILE_H
