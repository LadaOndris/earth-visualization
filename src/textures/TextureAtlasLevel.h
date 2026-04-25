
#pragma once

#include "Texture.h"
#include "../tiling/Tile.h"

#include <stdexcept>
#include <unordered_map>
#include <memory>

struct TileKey {
    int xIndex;
    int yIndex;
    bool operator==(const TileKey &) const = default;
};

struct TileKeyHash {
    size_t operator()(const TileKey &k) const {
        size_t seed = std::hash<int>{}(k.xIndex);
        seed ^= std::hash<int>{}(k.yIndex) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
        return seed;
    }
};

class TextureAtlasLevel {
public:
    TextureAtlasLevel(int xTiles, int yTiles)
        : _xTiles(xTiles), _yTiles(yTiles) {}

    void addTexture(std::shared_ptr<Texture> texture) {
        _textures[{texture->getXIndex(), texture->getYIndex()}] = std::move(texture);
    }

    std::shared_ptr<Texture> getTexture(const Tile &tile) const {
        const int xIndex = static_cast<int>((tile.getLongitude() + 180.0) / 360.0 * _xTiles);
        const int yIndex = static_cast<int>((tile.getLatitude()  +  90.0) / 180.0 * _yTiles);

        const auto it = _textures.find({xIndex, yIndex});
        if (it != _textures.end()) {
            return it->second;
        }

        throw std::runtime_error("Could not find texture for tile with longitude " + std::to_string(tile.getLongitude()) +
                                 " and latitude " + std::to_string(tile.getLatitude()));
    }

    int getXTiles() const { return _xTiles; }
    int getYTiles() const { return _yTiles; }

private:
    int _xTiles;
    int _yTiles;
    std::unordered_map<TileKey, std::shared_ptr<Texture>, TileKeyHash> _textures;
};
