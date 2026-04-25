//
// Created by lada on 10/17/23.
//

#ifndef EARTH_VISUALIZATION_TILERESOURCES_H
#define EARTH_VISUALIZATION_TILERESOURCES_H

#include "../textures/Texture.h"
#include "../vertex.h"
#include "Tile.h"
#include <utility>
#include <vector>
#include <glm/vec3.hpp>
#include <memory>

enum TextureType {
    Day, Night, HeightMap
};

class TileResources {
private:
    // Mesh covers always the tile only
    Mesh_t _mesh;
    // Textures may cover many tiles
    std::shared_ptr<Texture> _dayTexture;
    std::shared_ptr<Texture> _nightTexture;
    std::shared_ptr<Texture> _heightMap;
public:
    unsigned int meshVAO = 0, meshVBO = 0;
    // Coarser and finer resources form a hierarchical structure of the resources.
    std::weak_ptr<TileResources> coarserResources;
    std::vector<std::shared_ptr<TileResources>> finerResources;

    explicit TileResources(Mesh_t mesh, std::shared_ptr<Texture> dayTexture,
                           std::shared_ptr<Texture> nightTexture,
                           std::shared_ptr<Texture> heightMap) :
            _mesh(std::move(mesh)), _dayTexture(std::move(dayTexture)),
            _nightTexture(std::move(nightTexture)), _heightMap(std::move(heightMap)) {
    }

    [[nodiscard]] Mesh_t getMesh() const {
        return _mesh;
    }

    [[nodiscard]] std::shared_ptr<Texture> getTexture(TextureType textureType) const {
        switch (textureType) {
            case TextureType::Day: {
                return _dayTexture;
            }
            case TextureType::Night: {
                return _nightTexture;
            }
            case TextureType::HeightMap: {
                return _heightMap;
            }
            default:
                throw std::runtime_error("Invalid texture type");
        }
    }

    /**
     * @return True if a texture ready in OpenGL context was found.
     */
    [[nodiscard]] bool getCoarserTexture(std::shared_ptr<Texture> &texture, TextureType textureType) const {
        auto coarser = coarserResources.lock();
        if (!coarser) {
            return false;
        }
        auto coarserTexture = coarser->getTexture(textureType);
        if (coarserTexture->isPreparedInGlContext()) {
            texture = coarserTexture;
            return true;
        } else {
            return coarser->getCoarserTexture(texture, textureType);
        }
    }

    [[nodiscard]] bool getFinerTexture(
            const Tile &tile, std::shared_ptr<Texture> &texture, TextureType textureType) const {
        if (finerResources.empty()) {
            return false;
        }
        for (auto &finerResource: finerResources) {
            // Skip if the overlap of the tile and the resources in none.
            auto finerTexture = finerResource->getTexture(textureType);
            if (!tile.isTileWithinTexture(finerTexture)) {
                continue;
            }

            if (finerTexture->isPreparedInGlContext()) {
                texture = finerTexture;
                return true;
            } else {
                return finerResource->getFinerTexture(tile, texture, textureType);
            }
        }
        return false;
    }

};


#endif //EARTH_VISUALIZATION_TILERESOURCES_H
