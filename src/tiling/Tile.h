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
    std::vector<TileResources> lodResources;
    double latitude, longitude, latitudeWidth, longitudeWidth;

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

    /**
     * Select the appropriate level of detail (LOD) based on the calculated screen-space error.
     *
     * @param screenSpaceError The calculated screen-space error.
     * @return The index of the selected LOD.
     */
    int selectLOD(double screenSpaceError) {
        if (screenSpaceError < 1.0) {
            return 0; // Lowest LOD
        } else if (screenSpaceError < 2.0) {
            return 1; // Medium LOD
        } else {
            return 2; // Highest LOD
        }
    }

public:
    explicit Tile(double latitude, double longitude, double latitudeWidth, double longitudeWidth)
            : latitude(latitude), longitude(longitude), latitudeWidth(latitudeWidth), longitudeWidth(longitudeWidth) {
    }

    /**
     * Selects the tile resources for the appropriate level of detail.
     * The following parameters play a role in which LOD gets selected:
     * - the width of the screen-space (x)
     * - the width of the view-frustrum (w)
     * - distance from the center of the tile to the camera position (d)
     * - view angle of the camera (theta)
     * - geometric error of the object, given the specific texture (e)
     * We can use these quantities to compute the screen-space error and choose the LOD
     * with the lowest screen-space error.
     * @param cameraParams Parameters containing the angle of the camera
     * @return
     */
    TileResources getResources(double screenSpaceWidth, double viewFrustrumWidth, double distanceToCamera,
                               double cameraViewAngle, double geometricError) {
        // Calculate the screen-space error based on the provided parameters.
        double screenSpaceError = computeScreenSpaceError(screenSpaceWidth, viewFrustrumWidth, distanceToCamera, cameraViewAngle, geometricError);

        // Determine the appropriate level of detail (LOD) based on the screen-space error.
        int selectedLOD = selectLOD(screenSpaceError);

        // Return the resources for the selected LOD.
        return lodResources[selectedLOD];
    }

    void addResources(const TileResources &resources) {
        // Implement adding resources to the tile.
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
