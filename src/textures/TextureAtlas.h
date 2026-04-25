#pragma once

#include "Texture.h"
#include "TextureAtlasLevel.h"
#include "TextureAtlasReader.h"
#include "tiling/Tile.h"

#include <vector>
#include <filesystem>
#include <iostream>
#include <stdexcept>

namespace fs = std::filesystem;

class TextureAtlas {
public:
    void registerAvailableTextures(const fs::path &path) {
        TextureAtlasReader reader;
        auto levels = reader.read(path);
        if (!levels) {
            throw std::runtime_error("Failed to read texture atlas from " + path.string());
        }
        _levels = std::move(*levels);
        logRegisteredTextures(path);
    }

    int getNumLevelsOfDetail() const {
        return static_cast<int>(_levels.size());
    }

    Resolution getMostDetailedLevelDimensions() const {
        Resolution maxDimensions(0, 0);
        for (const auto &level : _levels) {
            Resolution levelDimensions(level.getXTiles(), level.getYTiles());
            if (levelDimensions > maxDimensions) {
                maxDimensions = levelDimensions;
            }
        }
        return maxDimensions;
    }

    Resolution getLevelDimensions(unsigned int level) const {
        assert(level < _levels.size());
        return Resolution(_levels[level].getXTiles(), _levels[level].getYTiles());
    }

    std::shared_ptr<Texture> getTexture(unsigned int level, const Tile &tile) const {
        if (level >= _levels.size()) {
            throw std::out_of_range("Requested level " + std::to_string(level) +
                " exceeds available levels (" + std::to_string(_levels.size()) + ").");
        }
        return _levels[level].getTexture(tile);
    }

private:
    std::vector<TextureAtlasLevel> _levels;

    void logRegisteredTextures(const fs::path &path) const {
        if (_levels.empty()) {
            std::cout << "No textures found in " << path << "\n";
            return;
        }
        for (size_t i = 0; i < _levels.size(); i++) {
            std::cout << "Level " << i << ": "
                      << _levels[i].getXTiles() << "x"
                      << _levels[i].getYTiles() << " y\n";
        }
    }
};
