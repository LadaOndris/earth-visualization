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

class SunRenderer : public Renderer {
private:
    Camera &camera;
    Shader shader;
    glm::vec3 lightPosition;
    float sunRadius;
    unsigned int VAO;
    unsigned int VBO;
    unsigned int EBO;

    std::vector<float> sunVertices;
    const static unsigned int sunIndices[];

    glm::vec3 sunLocation;

    void constructVertices();

    void setupVertexArrays();


public:
    SunRenderer(Camera &camera, glm::vec3 lightPosition, float sunRadius)
            : shader("shaders/sun/shader.vs", "shaders/sun/shader.fs"),
              camera(camera), lightPosition(lightPosition), sunRadius(sunRadius) {
    }

    bool initialize() override;

    void render(float currentTime, t_window_definition window, RenderingOptions options) override;

    void destroy() override;
};


#endif //EARTH_VISUALIZATION_SUNRENDERER_H
