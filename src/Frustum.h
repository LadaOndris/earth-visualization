
#pragma once

#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>
#include <glm/geometric.hpp>

#include <cmath>

class Frustum {
public:
    explicit Frustum(const glm::mat4 viewMatrix, const glm::mat4 projectionMatrix);

    [[nodiscard]] bool isPointOutside(glm::vec3 geocentricPoint) const;

    [[nodiscard]] bool intersectsEdge(std::pair<glm::vec3, glm::vec3> edge) const;

private:
    static const int numPlanes = 6;
    glm::vec4 planes[numPlanes];
};
