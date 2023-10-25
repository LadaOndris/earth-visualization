//
// Created by lada on 9/27/23.
//

#include "SunRenderer.h"

#include <cmath>

const unsigned int SunRenderer::sunIndices[] = {
        0, 1, 2,  // Triangle 1
};


bool SunRenderer::initialize() {
    constructVertices();
    setupVertexArrays();
    return true;
}


void SunRenderer::constructVertices() {
    auto radius = sunRadius;
    int numSegments = 360;

    sunVertices.push_back(0);
    sunVertices.push_back(0);
    sunVertices.push_back(0);

    for (int i = 0; i <= numSegments; ++i) {
        float theta = (2.0f * 3.14159265359f * static_cast<float>(i)) / static_cast<float>(numSegments);
        float x = radius * std::cos(theta);
        float z = radius * std::sin(theta);
        float y = 0;

        sunVertices.push_back(x);
        sunVertices.push_back(y);
        sunVertices.push_back(z);
    }
}

void SunRenderer::setupVertexArrays() {
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sunVertices.size() * sizeof(float),
                 sunVertices.data(), GL_STATIC_DRAW);

//    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
//    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(sunIndices), sunIndices, GL_STATIC_DRAW);

    // Set vertex attributes (e.g., position)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *) 0);
    glEnableVertexAttribArray(0);

    // Unbind VAO, VBO, and EBO
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void SunRenderer::render(float currentTime, t_window_definition window, RenderingOptions options) {
    glm::mat4 projectionMatrix;
    auto sunDistance = glm::length(lightPosition);
    auto minDepth = 1.f; //sunDistance - 3 * sunRadius;
    auto maxDepth = sunDistance + 3 * sunRadius;
    projectionMatrix = glm::perspective(glm::radians(camera.getFov()),
                                        (float) window.width / (float) window.height,
                                        minDepth, maxDepth);

    glm::vec3 direction = glm::normalize(lightPosition - glm::vec3(0.0f, 0.0f, 0.0f));
    // Angle between original XY plane and desired plane
    float angle = std::acos(glm::dot(glm::vec3(0.0f, 1.0f, 0.0f), direction));

    glm::mat4 rotationMatrix = glm::rotate(glm::mat4(1.0f), angle,
                                           glm::cross(glm::vec3(0.0f, 1.0f, 0.0f), direction));
    glm::mat4 translationMatrix = glm::translate(glm::mat4(1.0f), lightPosition);
    glm::mat4 modelMatrix = translationMatrix * rotationMatrix;

    shader.use();

    glBindVertexArray(VAO);

    // shader.setVec3("sunLocation", sunLocation);
    shader.setMat4("model", modelMatrix);
    shader.setMat4("projection", projectionMatrix);
    shader.setMat4("view", camera.getViewMatrix());

    glDrawArrays(GL_TRIANGLE_FAN, 0, 362);

//    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
//    glDrawElements(GL_TRIANGLES, vertices.size(), GL_FLOAT, 0);
}

void SunRenderer::destroy() {

}
