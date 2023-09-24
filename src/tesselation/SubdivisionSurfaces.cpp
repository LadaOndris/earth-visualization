//
// Created by lada on 9/24/23.
//

#include <glm/geometric.hpp>
#include "SubdivisionSurfaces.h"

SubdivisionSurfaces::SubdivisionSurfaces(Ellipsoid &ellipsoid) : ellipsoid(ellipsoid) {
    auto p0 = glm::vec3(0.f, 0.f, 1.f);
    auto p1 = glm::vec3(0.f, 2.f * std::sqrt(2.f), -1.f) / 3.f;
    auto p2 = glm::vec3(-std::sqrt(6.f), -std::sqrt(2.f), -1.f) / 3.f;
    auto p3 = glm::vec3(std::sqrt(6.f), -std::sqrt(2.f), -1.f) / 3.f;
    vertices.assign({p0, p1, p2, p0, p3, p2, p3, p2, p1, p0, p1, p3});
}

std::vector<glm::vec3> SubdivisionSurfaces::tessellate(int repetitions) {
    std::vector<glm::vec3> newVertices;

    for (int i = 0; i < repetitions; i++) {
        newVertices.clear();

        // Always get three subsequent vertices which define a single triangle
        for (int vertexIndex = 0; vertexIndex < vertices.size(); vertexIndex += 3) {
            auto p0 = vertices[vertexIndex];
            auto p1 = vertices[vertexIndex + 1];
            auto p2 = vertices[vertexIndex + 2];

            auto p01 = (p0 + p1) / 2.f;
            auto p12 = (p1 + p2) / 2.f;
            auto p20 = (p2 + p0) / 2.f;

            // Normalize points to lie on the unit sphere
            p01 = glm::normalize(p01);
            p12 = glm::normalize(p12);
            p20 = glm::normalize(p20);

            auto fourNewTriangles = {p0, p01, p20, p01, p1, p12, p01, p20, p12, p20, p12, p2};
            for (auto vertex : fourNewTriangles) {
                newVertices.push_back(vertex);
            }
        }
        vertices = newVertices;
    }
    return vertices;
}
