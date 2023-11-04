//
// Created by lada on 9/27/23.
//

#include "SunRenderer.h"

#include <cmath>

bool SunRenderer::initialize() {
    bool isShaderProgramBuilt = shader.build();
    if (!isShaderProgramBuilt) {
        return false;
    }

    constructVertices();
    setupVertexArrays();

    return true;
}


void SunRenderer::constructVertices() {
    sunVertices.push_back(0);
    sunVertices.push_back(0);
    sunVertices.push_back(0);

    for (int i = 0; i <= numSegments; ++i) {
        float theta = (2.0f * 3.14159265359f * static_cast<float>(i)) / static_cast<float>(numSegments);
        float x = sunRadius * std::cos(theta);
        float z = sunRadius * std::sin(theta);
        float y = 0;

        sunVertices.push_back(x);
        sunVertices.push_back(y);
        sunVertices.push_back(z);
    }
}

void SunRenderer::setupVertexArrays() {
    glCreateBuffers(1, &VBO);

    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glNamedBufferData(VBO, sunVertices.size() * sizeof(float),
                 &sunVertices.front(), GL_STATIC_DRAW);

    // Set vertex attributes (e.g., position)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *) 0);
    glEnableVertexAttribArray(0);

    GLenum error = glGetError();
    if (error != GL_NO_ERROR) {
        std::cerr << "[SunRenderer] OpenGL error after setting up vertex arrays: " << error << std::endl;
    }
    // Unbind VAO, VBO, and EBO
   // glBindBuffer(GL_ARRAY_BUFFER, 0);
   // glBindVertexArray(0);
}

void SunRenderer::render(float currentTime, t_window_definition window, RenderingOptions options) {
    glm::mat4 projectionMatrix;
    auto toSun = lightPosition - glm::vec3(0.0f, 0.0f, 0.0f);
    auto sunDistance = glm::length(toSun);
    auto minDepth = 1.f; //sunDistance - 3 * sunRadius;
    auto maxDepth = sunDistance + 3 * sunRadius;
    projectionMatrix = glm::perspective(glm::radians(camera.getFov()),
                                        (float) window.width / (float) window.height,
                                        minDepth, maxDepth);

    glm::vec3 direction = glm::normalize(toSun);

    // Angle between original XY plane and desired plane
    float angle = std::acos(glm::dot(glm::vec3(0.0f, 1.0f, 0.0f), direction));

    glm::mat4 rotationMatrix = glm::rotate(glm::mat4(1.0f), angle,
                                           glm::cross(glm::vec3(0.0f, 1.0f, 0.0f), direction));
    glm::mat4 translationMatrix = glm::translate(glm::mat4(1.0f), lightPosition);
    glm::mat4 modelMatrix = translationMatrix * rotationMatrix;

    GLenum error = glGetError();
    if (error != GL_NO_ERROR) {
        std::cerr << "[SunRenderer] OpenGL error before shader.use: " << error << std::endl;
    }
    shader.use();

    error = glGetError();
    if (error != GL_NO_ERROR) {
        std::cerr << "[SunRenderer] OpenGL error after shader.use: " << error << std::endl;
    }

    glBindVertexArray(VAO);


    error = glGetError();
    if (error != GL_NO_ERROR) {
        std::cerr << "[SunRenderer] OpenGL error after setting VAO: " << error << std::endl;
    }

    // shader.setVec3("sunLocation", sunLocation);
    shader.setMat4("model", modelMatrix);
    shader.setMat4("projection", projectionMatrix);
    shader.setMat4("view", camera.getViewMatrix());

    if (options.isWireframeEnabled) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    } else {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }
    glDrawArrays(GL_TRIANGLE_FAN, 0, sunVertices.size() / 3);


    error = glGetError();
    if (error != GL_NO_ERROR) {
        std::cerr << "[SunRenderer] OpenGL error after rendering: " << error << std::endl;
    }
//    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
//    glDrawElements(GL_TRIANGLES, vertices.size(), GL_FLOAT, 0);
}

void SunRenderer::destroy() {
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
}
