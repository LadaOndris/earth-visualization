//
// Created by lada on 9/27/23.
//

#include "SunRenderer.h"
#include "../tesselation/SubdivisionSphereTesselator.h"

#include <cmath>

bool SunRenderer::initialize() {
    bool isShaderProgramBuilt = program.build();
    if (!isShaderProgramBuilt) {
        return false;
    }

    constructVertices();
    setupVertexArrays();

    return true;
}


void SunRenderer::constructVertices() {
    SubdivisionSphereTesselator sphereTesselator;
    Mesh_t mesh = sphereTesselator.tessellate(4);

    for (auto &vertex : mesh) {
        vertex *= sunRadius;
    }

    sunVertices = convertToVertices(mesh);
}

void SunRenderer::setupVertexArrays() {
    glCreateBuffers(1, &VBO);

    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glNamedBufferData(VBO, sunVertices.size() * sizeof(t_vertex),
                      &sunVertices.front(), GL_STATIC_DRAW);

    // Set vertex attributes (e.g., position)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *) 0);
    glEnableVertexAttribArray(0);

    GLenum error = glGetError();
    if (error != GL_NO_ERROR) {
        std::cerr << "[SunRenderer] OpenGL error after setting up vertex arrays: " << error << std::endl;
    }
}

void SunRenderer::render(float currentTime, t_window_definition window, RenderingOptions options) {
    GLenum error = glGetError();
    if (error != GL_NO_ERROR) {
        std::cerr << "[SunRenderer] OpenGL error before program.use: " << error << std::endl;
    }
    program.use();

    error = glGetError();
    if (error != GL_NO_ERROR) {
        std::cerr << "[SunRenderer] OpenGL error after program.use: " << error << std::endl;
    }

    glBindVertexArray(VAO);

    error = glGetError();
    if (error != GL_NO_ERROR) {
        std::cerr << "[SunRenderer] OpenGL error after setting VAO: " << error << std::endl;
    }

    // program.setVec3("sunLocation", sunLocation);
    program.setMat4("model", getModelMatrix());
    program.setMat4("projection", getProjectionMatrix(window));
    program.setMat4("view", camera.getViewMatrix());

    if (options.isWireframeEnabled) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    } else {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }
    glDrawArrays(GL_TRIANGLES, 0, sunVertices.size());

    error = glGetError();
    if (error != GL_NO_ERROR) {
        std::cerr << "[SunRenderer] OpenGL error after rendering: " << error << std::endl;
    }
}

glm::mat4 SunRenderer::getProjectionMatrix(t_window_definition window) const {
    auto lightPosition = lightSource.getLightPosition();

    auto toSun = lightPosition - glm::vec3(0.0f, 0.0f, 0.0f);
    auto sunDistance = glm::length(toSun);
    auto minDepth = 1.f;
    auto maxDepth = sunDistance + 3 * sunRadius;
    glm::mat4 projectionMatrix = glm::perspective(glm::radians(camera.getFov()),
                                                  (float) window.width / (float) window.height,
                                                  minDepth, maxDepth);
    return projectionMatrix;
}

glm::mat4 SunRenderer::getModelMatrix() const {
    glm::mat4 modelMatrix = lightSource.getTransformationMatrix();
    return modelMatrix;
}

void SunRenderer::destroy() {
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
}
