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
    Mesh_t mesh;
    Texture &colorMap;
    Texture &heightMap;

    // Coarser and finer resources form a hierarchical structure of the resources.
    std::shared_ptr<TileResources> coarserResources;
    std::vector<std::shared_ptr<TileResources>> finerResources;
public:
    explicit TileResources(Mesh_t mesh, Texture &colorMap, Texture &heightMap) :
            mesh(std::move(mesh)), colorMap(colorMap), heightMap(heightMap) {

    }
};


#endif //EARTH_VISUALIZATION_TILERESOURCES_H
