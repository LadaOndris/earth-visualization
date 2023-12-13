
#include "Frustum.h"

Frustum::Frustum(const glm::mat4 viewMatrix, const glm::mat4 projectionMatrix) {
    glm::mat4 viewProjectionMatrix = projectionMatrix * viewMatrix;

    // Extract the frustum planes from the viewProjectionMatrix.
    // https://www8.cs.umu.se/kurser/5DV051/HT12/lab/plane_extraction.pdf
    for (int i = 0; i < 4; i++) {
        planes[0][i] = viewProjectionMatrix[i][3] + viewProjectionMatrix[i][0];  // Left
        planes[1][i] = viewProjectionMatrix[i][3] - viewProjectionMatrix[i][0];  // Right
        planes[2][i] = viewProjectionMatrix[i][3] + viewProjectionMatrix[i][1];  // Bottom
        planes[3][i] = viewProjectionMatrix[i][3] - viewProjectionMatrix[i][1];  // Top
        planes[4][i] = viewProjectionMatrix[i][3] + viewProjectionMatrix[i][2];  // Near
        planes[5][i] = viewProjectionMatrix[i][3] - viewProjectionMatrix[i][2];  // Far
    }
}

[[nodiscard]] bool Frustum::isPointOutside(glm::vec3 geocentricPoint) const {
    // Check if the tile's inside all frustum planes.
    for (auto plane : planes) {
        float signedDistance = glm::dot(plane, glm::vec4(geocentricPoint, 1.0));

        if (signedDistance < 0) {
            return true;
        }
    }
    return false;
}
[[nodiscard]] bool Frustum::intersectsEdge(std::pair<glm::vec3, glm::vec3> edge) const {
    for (auto plane : planes) {
        float d1 = glm::dot(plane, glm::vec4(edge.first, 1.0));
        float d2 = glm::dot(plane, glm::vec4(edge.second, 1.0));

        if ((d1 > 0 && d2 < 0) || (d1 < 0 && d2 > 0)) {
            return true;
        }
    }
    return false;
}