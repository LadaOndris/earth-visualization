//
// Created by lada on 10/17/23.
//

#ifndef EARTH_VISUALIZATION_TILERESOURCES_H
#define EARTH_VISUALIZATION_TILERESOURCES_H

#include "../textures/Texture.h"
#include <utility>
#include <vector>
#include <glm/vec3.hpp>
#include <memory>

typedef std::vector<glm::vec3> Mesh_t;

class TileResources {
private:
    // Mesh covers always the tile only
    Mesh_t mesh;
    // Textures may cover many tiles
    Texture &colorMap;
    Texture &heightMap;

    // Offset in VAO
    unsigned int meshBufferOffset = -1;
public:
    unsigned int meshVAO, meshVBO;
    // Coarser and finer resources form a hierarchical structure of the resources.
    std::shared_ptr<TileResources> coarserResources;
    std::vector<std::shared_ptr<TileResources>> finerResources;

    explicit TileResources(Mesh_t mesh, Texture &colorMap, Texture &heightMap) :
            mesh(std::move(mesh)), colorMap(colorMap), heightMap(heightMap) {

    }

    Mesh_t getMesh() const {
        return mesh;
    }

    Texture &getDayTexture() const {
        return colorMap;
    }

    Texture &getHeightMapTexture() const {
        return heightMap;
    }

    void setMeshBufferOffset(unsigned int offsetInVertices) {
        // Offset in number of vertices
        meshBufferOffset = offsetInVertices;
    }

    /**
     * Returns offset in the vertex buffer in the number of vertices.
     */
    unsigned int getMeshBufferOffset() const {
        return meshBufferOffset;
    }

};


#endif //EARTH_VISUALIZATION_TILERESOURCES_H