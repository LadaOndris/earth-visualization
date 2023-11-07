//
// Created by lada on 9/27/23.
//

#ifndef EARTH_VISUALIZATION_SUNRENDERER_H
#define EARTH_VISUALIZATION_SUNRENDERER_H


#include <vector>
#include "Renderer.h"
#include "shader.h"
#include "../cameras/FreeCamera.h"
#include "../vertex.h"
#include "../simulation/LightSource.h"

class SunRenderer : public Renderer {
private:
    int numSegments = 36;
    Camera &camera;
    const LightSource &lightSource;
    Shader shader;
    float sunRadius;
    unsigned int VAO;
    unsigned int VBO;

    std::vector<t_vertex> sunVertices;

    glm::vec3 sunLocation;

    void constructVertices();

    void setupVertexArrays();


public:
    explicit SunRenderer(Camera &camera, const LightSource &lightSource, float sunRadius)
            : shader("shaders/sun/shader.vert", "shaders/sun/shader.frag"),
              camera(camera), lightSource(lightSource), sunRadius(sunRadius) {
    }

    bool initialize() override;

    void render(float currentTime, t_window_definition window, RenderingOptions options) override;

    void destroy() override;

    [[nodiscard]] glm::mat4 getProjectionMatrix(t_window_definition window) const;

    [[nodiscard]] glm::mat4 getModelMatrix() const;
};


#endif //EARTH_VISUALIZATION_SUNRENDERER_H
