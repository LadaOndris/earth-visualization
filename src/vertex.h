
#pragma once

#include <glm/vec3.hpp>

#include <vector>

typedef struct {
    float x, y, z;
} t_vertex;

typedef std::vector<glm::vec3> Mesh_t;

std::vector<t_vertex> convertToVertices(const std::vector<glm::vec3> &projectedVertices);
