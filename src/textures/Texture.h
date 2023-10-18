//
// Created by lada on 10/17/23.
//

#ifndef EARTH_VISUALIZATION_TEXTURE_H
#define EARTH_VISUALIZATION_TEXTURE_H

#include <iostream>
#include <utility>
#include <GL/glew.h>
#include <stb_image.h>
#include "../tiling/Resolution.h"

class Texture {
private:
    std::string path;
    bool _isLoaded = false;
    unsigned char *data = nullptr;
    Resolution resolution = Resolution(0, 0);
public:
    explicit Texture(std::string path) : path(std::move(path)) {

    }

    void load() {
        if (_isLoaded) {
            return; // Texture is already loaded.
        }
        int width, height;
        stbi_set_flip_vertically_on_load(true);
        data = stbi_load(path.c_str(), &width, &height, nullptr, 0);

        if (data) {
            resolution = Resolution(width, height);
            _isLoaded = true;
        }
        else {
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
        resolution = Resolution(0, 0);
    }

    bool isLoaded() const {
        return _isLoaded;
    }

    std::string getPath() const {
        return path;
    }

};

#endif //EARTH_VISUALIZATION_TEXTURE_H
