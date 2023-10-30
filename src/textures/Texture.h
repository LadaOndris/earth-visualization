//
// Created by lada on 10/17/23.
//

#ifndef EARTH_VISUALIZATION_TEXTURE_H
#define EARTH_VISUALIZATION_TEXTURE_H

#include <iostream>
#include <utility>
#include <stb_image.h>
#include <glm/vec2.hpp>
#include "../tiling/Resolution.h"

class Texture {
private:
    std::string path;
    bool _isLoaded = false;
    unsigned char *data = nullptr;
    Resolution resolution; // Resolution in pixels
    glm::vec2 geodeticOffset; // Offset of this texture on the ellipsoid
    glm::vec2 geodeticSize; // Width in longitude and latitude
    glm::vec2 textureGridSize;
public:
    unsigned int textureId;

    explicit Texture(std::string path, int width,
                     glm::vec2 geodeticOffset, glm::vec2 geodeticSize,
                     glm::vec2 textureGridSize)
            : path(std::move(path)),
              resolution(width, width),
              geodeticOffset(geodeticOffset),
              geodeticSize(geodeticSize),
              textureGridSize(textureGridSize) {
    }

    void load() {
        if (_isLoaded) {
            return; // Texture is already loaded.
        }
        int width, height, channels;
        // stbi_set_flip_vertically_on_load(true);
        data = stbi_load(path.c_str(), &width, &height, &channels, 0);

        if (data) {
            _isLoaded = true;
        } else {
            std::cout << "Failed to load texture: " << path << std::endl;
        }
    }

    void freeData() {
        stbi_image_free(data);
        data = nullptr;
    }

    void unload() {
        if (!_isLoaded) {
            return; // Texture is not loaded.
        }

        stbi_image_free(data);
        data = nullptr;
        _isLoaded = false;
    }

    [[nodiscard]] bool isLoaded() const {
        return _isLoaded;
    }

    [[nodiscard]] std::string getPath() const {
        return path;
    }

    [[nodiscard]] Resolution getResolution() const {
        return resolution;
    }

    [[nodiscard]] unsigned char *getData() const {
        return data;
    }

    [[nodiscard]] double getLatitudeWidth() const {
        return geodeticSize[1];
    }

    [[nodiscard]] double getLongitudeWidth() const {
        return geodeticSize[0];
    }

    [[nodiscard]] glm::vec2 getGeodeticOffset() const {
        return geodeticOffset;
    }

    [[nodiscard]] glm::vec2 getTextureGridSize() const {
        return textureGridSize;
    }

    [[nodiscard]] unsigned int &getTextureId() {
        return textureId;
    }
};

#endif //EARTH_VISUALIZATION_TEXTURE_H
