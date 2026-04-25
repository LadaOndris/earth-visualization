#pragma once

#include "Renderer.h"
#include "cameras/Camera.h"
#include "program.h"
#include "rendering/MeshBuffer.h"
#include "simulation/LightSource.h"

class SunRenderer : public Renderer {
public:
    explicit SunRenderer(Camera &camera, const LightSource &lightSource, float sunRadius,
                         Program program);

    SunRenderer(const SunRenderer&) = delete;
    SunRenderer& operator=(const SunRenderer&) = delete;

    void render(float currentTime, t_window_definition window, RenderingOptions options) override;

    [[nodiscard]] glm::mat4 getProjectionMatrix(t_window_definition window) const;

    [[nodiscard]] glm::mat4 getModelMatrix() const;

private:
    Program _program;
    Camera &_camera;
    const LightSource &_lightSource;
    float _sunRadius;
    MeshBuffer _meshBuffer;
};
