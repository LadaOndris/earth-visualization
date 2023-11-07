//
// Created by lada on 10/17/23.
//

#ifndef EARTH_VISUALIZATION_TILERESOURCES_H
#define EARTH_VISUALIZATION_TILERESOURCES_H

#include "../textures/Texture.h"
#include "../vertex.h"
#include <utility>
#include <vector>
#include <glm/vec3.hpp>
#include <memory>

class TileResources {
private:
    // Mesh covers always the tile only
    Mesh_t mesh;
    // Textures may cover many tiles
    std::shared_ptr<Texture> colorMap;
    std::shared_ptr<Texture> heightMap;
public:
    unsigned int meshVAO, meshVBO;
    // Coarser and finer resources form a hierarchical structure of the resources.
    std::shared_ptr<TileResources> coarserResources;
    std::vector<std::shared_ptr<TileResources>> finerResources;

    explicit TileResources(Mesh_t mesh, std::shared_ptr<Texture> colorMap, std::shared_ptr<Texture> heightMap) :
            mesh(std::move(mesh)), colorMap(colorMap), heightMap(heightMap) {

    }

    [[nodiscard]] Mesh_t getMesh() const {
        return mesh;
    }

    [[nodiscard]] std::shared_ptr<Texture> getDayTexture() const {
        return colorMap;
    }

    /**
     *
     * @return True if a texture ready in OpenGL context was found.
     */
    [[nodiscard]] bool getCoarserDayTexture(std::shared_ptr<Texture> &texture) const {
        if (coarserResources == nullptr) {
            std::cout << mesh.size() << std::endl;
            return false;
        }
        auto coarserTexture = coarserResources->getDayTexture();
        if (coarserTexture->isPreparedInGlContext()) {
            texture = coarserTexture;
            return true;
        } else {
            return coarserResources->getCoarserDayTexture(texture);
        }
    }

    [[nodiscard]] std::shared_ptr<Texture> getHeightMapTexture() const {
        return heightMap;
    }

};


#endif //EARTH_VISUALIZATION_TILERESOURCES_H
