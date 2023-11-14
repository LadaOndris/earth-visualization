
#ifndef EARTH_VISUALIZATION_VERTEX_H
#define EARTH_VISUALIZATION_VERTEX_H

#include <vector>
#include <glm/vec3.hpp>

typedef struct {
    float x, y, z;
} t_vertex;

typedef std::vector<glm::vec3> Mesh_t;

std::vector<t_vertex> convertToVertices(const std::vector<glm::vec3> &projectedVertices);

#endif //EARTH_VISUALIZATION_VERTEX_H
