//
// Created by lada on 9/27/23.
//

#ifndef EARTH_VISUALIZATION_SUNRENDERER_H
#define EARTH_VISUALIZATION_SUNRENDERER_H


#include <vector>
#include "Renderer.h"
#include "program.h"
#include "../cameras/FreeCamera.h"
#include "../vertex.h"
#include "../simulation/LightSource.h"

class SunRenderer : public Renderer {
public:
    explicit SunRenderer(Camera &camera, const LightSource &lightSource, float sunRadius,
                         Program &program)
            : _program(program),
              _camera(camera), 
              _lightSource(lightSource), 
              _sunRadius(sunRadius) {
    }

    bool initialize() override;

    void render(float currentTime, t_window_definition window, RenderingOptions options) override;

    void destroy() override;

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


#endif //EARTH_VISUALIZATION_SUNRENDERER_H
