

#ifndef EARTH_VISUALIZATION_TEXTURE_H
#define EARTH_VISUALIZATION_TEXTURE_H

#include "tiling/Resolution.h"
#include "glad/glad.h"

#include <stb_image.h>
#include <glm/vec2.hpp>

#include <iostream>
#include <utility>
#include <vector>

class Texture {
private:
    std::string _path;
    Resolution _resolution; // Resolution in pixels
    glm::vec2 _geodeticOffset; // Offset of this texture on the ellipsoid
    glm::vec2 _geodeticSize; // Width in longitude and latitude
    glm::vec2 _textureGridSize;
    int _channels;
    int _xIndex;
    int _yIndex;

    bool _isGlPrepared = false;
    std::vector<unsigned char> _data;

    unsigned int _textureId;

    void freeData() {
        _data.clear();
    }

public:
    explicit Texture(std::string path, int width,
                     glm::vec2 geodeticOffset, glm::vec2 geodeticSize,
                     glm::vec2 textureGridSize, int xIndex, int yIndex)
            : _path(std::move(path)),
              _resolution(width, width),
              _geodeticOffset(geodeticOffset),
              _geodeticSize(geodeticSize),
              _textureGridSize(textureGridSize),
              _channels(0),
              _xIndex(xIndex),
              _yIndex(yIndex) {
    }

    void setData(std::vector<unsigned char> dataOther) {
        _data = std::move(dataOther);
    }

    void setChannels(int channelsValue) {
        _channels = channelsValue;
    }

    void loadIntoGL() {
        assert(!_data.empty());
        assert(!_isGlPrepared);

        auto width = _resolution.getWidth();
        auto height = _resolution.getHeight();

        int dataFormat = GL_RGB;
        int storageFormat = GL_RGB8;
        if (_channels == 1) {
            dataFormat = GL_RED;
            storageFormat = GL_R8;
        }

        glCreateTextures(GL_TEXTURE_2D, 1, &_textureId);

        //Check for OpenGL errors
        GLenum error = glGetError();
        if (error != GL_NO_ERROR) {
            std::cerr << "OpenGL error after glGenTextures: " << error << std::endl;
        }

        glTextureParameteri(_textureId, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTextureParameteri(_textureId, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTextureParameteri(_textureId, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTextureParameteri(_textureId, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        // Anisotropic filtering improves the appearance of textures
        // viewed at oblique angles, rather than straight-on.
        glTextureParameterf(_textureId, GL_TEXTURE_MAX_ANISOTROPY, 4);

        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        glTextureStorage2D(_textureId, 1, storageFormat, width, height);
        glTextureSubImage2D(_textureId, 0, 0, 0, width, height, dataFormat, GL_UNSIGNED_BYTE, _data.data());
        glGenerateTextureMipmap(_textureId);

        // Check for OpenGL errors after texture data loading
        error = glGetError();
        if (error != GL_NO_ERROR) {
            std::cerr << "OpenGL error after texture data loading: " << error << std::endl;
        }

        freeData();

        _isGlPrepared = true;
    }

    void unloadFromGL() {
        if (_isGlPrepared) {
            glDeleteTextures(1, &_textureId);
            _isGlPrepared = false;
        }
    }

    [[nodiscard]] bool isPreparedInGlContext() const {
        return _isGlPrepared;
    }

    [[nodiscard]] bool isLoaded() const {
        return !_data.empty();
    }

    [[nodiscard]] std::string getPath() const {
        return _path;
    }

    [[nodiscard]] Resolution getResolution() const {
        return _resolution;
    }

    [[nodiscard]] double getLatitudeWidth() const {
        return _geodeticSize[1];
    }

    [[nodiscard]] double getLongitudeWidth() const {
        return _geodeticSize[0];
    }

    [[nodiscard]] glm::vec2 getGeodeticOffset() const {
        return _geodeticOffset;
    }

    [[nodiscard]] glm::vec2 getTextureGridSize() const {
        return _textureGridSize;
    }

    [[nodiscard]] unsigned int &getTextureId() {
        return _textureId;
    }

    [[nodiscard]] int getXIndex() const {
        return _xIndex;
    }

    [[nodiscard]] int getYIndex() const {
        return _yIndex;
    }
};

#endif //EARTH_VISUALIZATION_TEXTURE_H
