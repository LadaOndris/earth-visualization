#pragma once

#include "Renderer.h"
#include "program.h"
#include "cameras/FreeCamera.h"
#include "vertex.h"
#include "simulation/LightSource.h"

#include <vector>

class SunRenderer : public Renderer {
public:
    explicit SunRenderer(Camera &camera, const LightSource &lightSource, float sunRadius,
                         Program &program);

    ~SunRenderer() override;

    SunRenderer(const SunRenderer&) = delete;
    SunRenderer& operator=(const SunRenderer&) = delete;

    void render(float currentTime, t_window_definition window, RenderingOptions options) override;

    [[nodiscard]] glm::mat4 getProjectionMatrix(t_window_definition window) const;

    [[nodiscard]] glm::mat4 getModelMatrix() const;

private:
    Program &_program;
    Camera &_camera;
    const LightSource &_lightSource;
    float _sunRadius;

    int numSegments = 36;
    unsigned int VAO;
    unsigned int VBO;

    std::vector<t_vertex> sunVertices;

    glm::vec3 sunLocation;

    void constructVertices();

    void setupVertexArrays();
};
