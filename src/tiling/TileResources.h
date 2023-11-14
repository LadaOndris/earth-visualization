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
    Mesh_t mesh;
    // Textures may cover many tiles
    std::shared_ptr<Texture> dayTexture;
    std::shared_ptr<Texture> nightTexture;
    std::shared_ptr<Texture> heightMap;
public:
    unsigned int meshVAO = 0, meshVBO = 0;
    // Coarser and finer resources form a hierarchical structure of the resources.
    std::shared_ptr<TileResources> coarserResources;
    std::vector<std::shared_ptr<TileResources>> finerResources;

    explicit TileResources(Mesh_t mesh, std::shared_ptr<Texture> dayTexture,
                           std::shared_ptr<Texture> nightTexture,
                           std::shared_ptr<Texture> heightMap) :
            mesh(std::move(mesh)), dayTexture(std::move(dayTexture)),
            nightTexture(std::move(nightTexture)), heightMap(std::move(heightMap)) {
    }

    [[nodiscard]] Mesh_t getMesh() const {
        return mesh;
    }

    [[nodiscard]] std::shared_ptr<Texture> getTexture(TextureType textureType) const {
        switch (textureType) {
            case TextureType::Day: {
                return dayTexture;
            }
            case TextureType::Night: {
                return nightTexture;
            }
            case TextureType::HeightMap: {
                return heightMap;
            }
            default:
                throw std::runtime_error("Invalid texture type");
        }
    }

    /**
     * @return True if a texture ready in OpenGL context was found.
     */
    [[nodiscard]] bool getCoarserTexture(std::shared_ptr<Texture> &texture, TextureType textureType) const {
        if (coarserResources == nullptr) {
            return false;
        }
        auto coarserTexture = coarserResources->getTexture(textureType);
        if (coarserTexture->isPreparedInGlContext()) {
            texture = coarserTexture;
            return true;
        } else {
            return coarserResources->getCoarserTexture(texture, textureType);
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
