//
// Created by lada on 9/26/23.
//

#ifndef EARTH_VISUALIZATION_EARTHRENDERER_H
#define EARTH_VISUALIZATION_EARTHRENDERER_H


#include <vector>
#include <glm/ext/matrix_float4x4.hpp>
#include "../include/glad/glad.h"
#include "../vertex.h"
#include "shader.h"
#include "../cameras/FreeCamera.h"
#include "../ellipsoid.h"
#include "../tesselation/SubdivisionSphereTesselator.h"
#include "Renderer.h"
#include "../window_definition.h"
#include <glm/vec3.hpp> // glm::vec3
#include <glm/vec4.hpp> // glm::vec4
#include <glm/mat4x4.hpp> // glm::mat4
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

class EarthRenderer : public Renderer {
private:
    SubdivisionSphereTesselator &subdivisionSurfaces;
    Camera &camera;
    Ellipsoid &ellipsoid;
    Shader shader;
    std::vector<t_vertex> vertices;
    glm::vec3 lightPosition;
    unsigned int VAO;
    unsigned int VBO;
    unsigned int dayTexture;
    unsigned int nightTexture;

    static void loadTexture(unsigned int &textureID, const std::string &texturePath);

    std::vector<t_vertex> convertToVertices(const std::vector<glm::vec3> &projectedVertices);

    void constructVertices(SphereTesselator &tesselator);

    void setupVertexArrays();

    void loadTextures(std::string dayTextureName, std::string nightTextureName);

public:
    EarthRenderer(SubdivisionSphereTesselator &subdivisionSurfaces, Ellipsoid &ellipsoid, Camera &camera,
                  glm::vec3 lightPosition)
            : shader("shaders/shader.vs", "shaders/shader.fs"), subdivisionSurfaces(subdivisionSurfaces),
              camera(camera), ellipsoid(ellipsoid), lightPosition(lightPosition) {
    }

    bool initialize() override;

    void destroy() override;

    void render(float currentTime, t_window_definition window, RenderingOptions options) override;
};


#endif //EARTH_VISUALIZATION_EARTHRENDERER_H
