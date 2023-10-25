//
// Created by lada on 10/17/23.
//

#ifndef EARTH_VISUALIZATION_TEXTURE_H
#define EARTH_VISUALIZATION_TEXTURE_H

#include <iostream>
#include <utility>
#include <stb_image.h>
#include "../tiling/Resolution.h"

class Texture {
private:
    std::string path;
    bool _isLoaded = false;
    unsigned char *data = nullptr;
    Resolution resolution;
    double latitudeWidth;
    double longitudeWidth;
public:
    explicit Texture(std::string path, int width, double latitudeWidth,
                     double longtitudeWidth)
            : path(std::move(path)),
              resolution(width, width),
              longitudeWidth(longtitudeWidth),
              latitudeWidth(latitudeWidth) {
    }

    void load() {
        if (_isLoaded) {
            return; // Texture is already loaded.
        }
        std::cout << "Loading texture." << std::endl;
        int width, height;
        stbi_set_flip_vertically_on_load(true);
        data = stbi_load(path.c_str(), &width, &height, nullptr, 0);

        if (data) {
            _isLoaded = true;
        } else {
            std::cout << "Failed to load texture: " << path << std::endl;
        }
    }

    void unload() {
        if (!_isLoaded) {
            return; // Texture is not loaded.
        }

        stbi_image_free(data);
        data = nullptr;
        _isLoaded = false;
    }

    bool isLoaded() const {
        return _isLoaded;
    }

    std::string getPath() const {
        return path;
    }

    Resolution getResolution() const {
        return resolution;
    }

    unsigned char *getData() const {
        return data;
    }

    double getLatitudeWidth() const {
        return latitudeWidth;
    }

    double getLongitudeWidth() const {
        return longitudeWidth;
    }

};

#endif //EARTH_VISUALIZATION_TEXTURE_H
