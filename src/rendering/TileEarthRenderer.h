//
// Created by lada on 10/19/23.
//

#ifndef EARTH_VISUALIZATION_TILEEARTHRENDERER_H
#define EARTH_VISUALIZATION_TILEEARTHRENDERER_H


#include <glm/vec3.hpp>
#include "Renderer.h"
#include "../cameras/Camera.h"
#include "../ellipsoid.h"
#include "../tiling/TileContainer.h"
#include "shader.h"
#include "../vertex.h"
#include "RendererSubscriber.h"

class TileEarthRenderer : public Renderer {
private:
    TileContainer &tileContainer;
    Camera &camera;
    Ellipsoid &ellipsoid;
    glm::vec3 lightPosition;
    Shader shader;
    std::vector<std::shared_ptr<RendererSubscriber>> subscribers;

    unsigned int dayTextureId;

    void initVertexArraysForAllLevels(int numLevels);

    void setupVertexArray(std::vector<t_vertex> vertices,
                          unsigned int &VAO, unsigned int &VBO);

    void prepareTexture(Texture &texture);

    glm::mat4 setupMatrices(float currentTime, t_window_definition window);

public:
    explicit TileEarthRenderer(TileContainer &tileContainer, Ellipsoid &ellipsoid, Camera &camera,
                               glm::vec3 lightPosition)
            : tileContainer(tileContainer), camera(camera), ellipsoid(ellipsoid), lightPosition(lightPosition),
              shader("shaders/tiling/shader.vs", "shaders/tiling/shader.fs") {
    }

    void render(float currentTime, t_window_definition window, RenderingOptions options) override;

    bool initialize() override;

    void destroy() override;

    /**
     * Adds a subscriber which wants to be notified
     * of the rendering results.
     */
    void addSubscriber(const std::shared_ptr<RendererSubscriber>& subscriber);

};


#endif //EARTH_VISUALIZATION_TILEEARTHRENDERER_H
