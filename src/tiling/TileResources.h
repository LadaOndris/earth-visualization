
#pragma once

#include "rendering/MeshBuffer.h"
#include "textures/Texture.h"
#include "vertex.h"
#include "Tile.h"

#include <glm/vec3.hpp>

#include <utility>
#include <vector>
#include <memory>

enum TextureType {
    Day, Night, HeightMap
};

class MeshBuffer;

class TileResources {
private:
    // Textures may cover many tiles
    std::shared_ptr<Texture> _dayTexture;
    std::shared_ptr<Texture> _nightTexture;
    std::shared_ptr<Texture> _heightMap;
    std::shared_ptr<MeshBuffer> _meshBuffer;
public:
    // Coarser and finer resources form a hierarchical structure of the resources.
    std::weak_ptr<TileResources> coarserResources;
    std::vector<std::shared_ptr<TileResources>> finerResources;

    explicit TileResources(
        std::shared_ptr<Texture> dayTexture,
        std::shared_ptr<Texture> nightTexture,
        std::shared_ptr<Texture> heightMap,
        std::shared_ptr<MeshBuffer> meshBuffer) :
            _dayTexture(std::move(dayTexture)),
            _nightTexture(std::move(nightTexture)), 
            _heightMap(std::move(heightMap)),
            _meshBuffer(std::move(meshBuffer)) {
    }

    [[nodiscard]] unsigned int getMeshVao() const;

    [[nodiscard]] const Mesh_t& getMesh() const {
        return _meshBuffer->getMesh();
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
