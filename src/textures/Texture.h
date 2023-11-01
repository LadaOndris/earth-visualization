//
// Created by lada on 10/17/23.
//

#ifndef EARTH_VISUALIZATION_TEXTURE_H
#define EARTH_VISUALIZATION_TEXTURE_H

#include <iostream>
#include <utility>
#include <stb_image.h>
#include <glm/vec2.hpp>
#include <vector>
#include "../tiling/Resolution.h"
#include "../include/glad/glad.h"

class Texture {
private:
    bool isGlPrepared = false;
    std::string path;
    std::vector<unsigned char> data;
    Resolution resolution; // Resolution in pixels
    glm::vec2 geodeticOffset; // Offset of this texture on the ellipsoid
    glm::vec2 geodeticSize; // Width in longitude and latitude
    glm::vec2 textureGridSize;

    unsigned int textureId;

    void freeData() {
        data.clear();
    }

public:
    explicit Texture(std::string path, int width,
                     glm::vec2 geodeticOffset, glm::vec2 geodeticSize,
                     glm::vec2 textureGridSize)
            : path(std::move(path)),
              resolution(width, width),
              geodeticOffset(geodeticOffset),
              geodeticSize(geodeticSize),
              textureGridSize(textureGridSize) {
    }

    void setData(std::vector<unsigned char> dataOther) {
        data = std::move(dataOther);
    }

    void loadIntoGL() {
        assert(!data.empty());
        assert(!isGlPrepared);

        auto width = resolution.getWidth();
        auto height = resolution.getHeight();

        glCreateTextures(GL_TEXTURE_2D, 1, &textureId);

        //Check for OpenGL errors
        GLenum error = glGetError();
        if (error != GL_NO_ERROR) {
            std::cerr << "OpenGL error after glGenTextures: " << error << std::endl;
        }

        glTextureParameteri(textureId, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTextureParameteri(textureId, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTextureParameteri(textureId, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTextureParameteri(textureId, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        glTextureStorage2D(textureId, 1, GL_RGB8, width, height);
        glTextureSubImage2D(textureId, 0, 0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, data.data());
        glGenerateTextureMipmap(textureId);

        // Check for OpenGL errors after texture data loading
        error = glGetError();
        if (error != GL_NO_ERROR) {
            std::cerr << "OpenGL error after texture data loading: " << error << std::endl;
        }

        freeData();

        isGlPrepared = true;
    }

    void unloadFromGL() {
        if (isGlPrepared) {
            glDeleteTextures(1, &textureId);
            isGlPrepared = false;
        }
    }

    [[nodiscard]] bool isPreparedInGlContext() const {
        return isGlPrepared;
    }

    [[nodiscard]] bool isLoaded() const {
        return !data.empty();
    }

    [[nodiscard]] std::string getPath() const {
        return path;
    }

    [[nodiscard]] Resolution getResolution() const {
        return resolution;
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
