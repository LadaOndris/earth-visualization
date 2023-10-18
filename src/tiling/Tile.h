//
// Created by lada on 10/17/23.
//

#ifndef EARTH_VISUALIZATION_TILE_H
#define EARTH_VISUALIZATION_TILE_H

#include <vector>
#include "TileResources.h"

class CameraParams {

};

class Tile {
private:
    std::vector<TileResources> lodResources;
    double latitude, longitude, latitudeWidth, longitudeWidth;

public:
    explicit Tile(double latitude, double longitude, double latitudeWidth, double longitudeWidth)
            : latitude(latitude), longitude(longitude), latitudeWidth(latitudeWidth), longitudeWidth(longitudeWidth) {
    }

    TileResources getResources(const CameraParams &cameraParams) {
        // Implement logic to select the appropriate resources for the tile based on camera parameters.
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
