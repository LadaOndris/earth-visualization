

#include "SunRenderer.h"
#include "tesselation/SubdivisionSphereTesselator.h"

#include <cmath>

namespace 
{

Mesh_t createMesh(float sunRadius) {
    SubdivisionSphereTesselator sphereTesselator;
    Mesh_t mesh = sphereTesselator.tessellate(4);

    for (auto &vertex : mesh) {
        vertex *= sunRadius;
    }
    return mesh;
}

}

SunRenderer::SunRenderer(Camera &camera, const LightSource &lightSource, float sunRadius,
                         Program program)
        : _program(std::move(program)),
          _camera(camera),
          _lightSource(lightSource),
          _sunRadius(sunRadius),
          _meshBuffer(createMesh(sunRadius)) {
}


void SunRenderer::render(float currentTime, t_window_definition window, RenderingOptions options) {
    GLenum error = glGetError();
    if (error != GL_NO_ERROR) {
        std::cerr << "[SunRenderer] OpenGL error before program.use: " << error << std::endl;
    }
    _program.use();

    error = glGetError();
    if (error != GL_NO_ERROR) {
        std::cerr << "[SunRenderer] OpenGL error after program.use: " << error << std::endl;
    }

    glBindVertexArray(_meshBuffer.getVao());

    error = glGetError();
    if (error != GL_NO_ERROR) {
        std::cerr << "[SunRenderer] OpenGL error after setting VAO: " << error << std::endl;
    }

    _program.setMat4("model", getModelMatrix());
    _program.setMat4("projection", getProjectionMatrix(window));
    _program.setMat4("view", _camera.getViewMatrix());

    if (options.isWireframeEnabled) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    } else {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }
    glDrawArrays(GL_TRIANGLES, 0, _meshBuffer.getMesh().size());

    error = glGetError();
    if (error != GL_NO_ERROR) {
        std::cerr << "[SunRenderer] OpenGL error after rendering: " << error << std::endl;
    }
}

glm::mat4 SunRenderer::getProjectionMatrix(t_window_definition window) const {
    auto lightPosition = _lightSource.getLightPosition();

    auto toSun = lightPosition - glm::vec3(0.0f, 0.0f, 0.0f);
    auto sunDistance = glm::length(toSun);
    auto minDepth = 1.f;
    auto maxDepth = sunDistance + 3 * _sunRadius;
    glm::mat4 projectionMatrix = glm::perspective(glm::radians(_camera.getFov()),
                                                  static_cast<float>(window.width) / static_cast<float>(window.height),
                                                  minDepth, maxDepth);
    return projectionMatrix;
}

glm::mat4 SunRenderer::getModelMatrix() const {
    glm::mat4 modelMatrix = _lightSource.getTransformationMatrix();
    return modelMatrix;
}

