//
// Created by lada on 10/19/23.
//

#ifndef EARTH_VISUALIZATION_TILEEARTHRENDERER_H
#define EARTH_VISUALIZATION_TILEEARTHRENDERER_H


#include <glm/vec3.hpp>
#include <unordered_map>
#include "Renderer.h"
#include "cameras/Camera.h"
#include "ellipsoid.h"
#include "tiling/TileContainer.h"
#include "program.h"
#include "vertex.h"
#include "RendererSubscriber.h"
#include "resources/ResourceFetcher.h"
#include "resources/ResourceManager.h"
#include "simulation/LightSource.h"

class TileEarthRenderer : public Renderer {
public:
    explicit TileEarthRenderer(TileContainer &tileContainer,
                               Ellipsoid &ellipsoid,
                               Camera &camera,
                               LightSource &lightSource,
                               ResourceFetcher &resourceFetcher,
                               ResourceManager &resourceManager,
                               Program &program)
            : _tileContainer(tileContainer), 
              _ellipsoid(ellipsoid),
              _camera(camera), 
              _lightSource(lightSource), 
              _resourceFetcher(resourceFetcher),
              _resourceManager(resourceManager),
              _program(program) {
    }

    void render(float currentTime, t_window_definition window, RenderingOptions options) override;

    bool initialize() override;

    void destroy() override;

    /**
     * Adds a subscriber which wants to be notified
     * of the rendering results.
     */
    void addSubscriber(const std::shared_ptr<RendererSubscriber>& subscriber);

private:
    TileContainer &_tileContainer;
    Ellipsoid &_ellipsoid;
    Camera &_camera;
    const LightSource &_lightSource;
    ResourceFetcher &_resourceFetcher;
    ResourceManager &_resourceManager;
    Program &_program;
    std::vector<std::shared_ptr<RendererSubscriber>> _subscribers;
    std::unordered_map<std::string, std::shared_ptr<Texture>> _requestMap;


    void initVertexArraysForAllLevels(int numLevels);

    void setupVertexArray(std::vector<t_vertex> vertices,
                          unsigned int &VAO, unsigned int &VBO);

    bool prepareTexture(const std::shared_ptr<Texture>& texture);

    Frustum setupMatrices(float currentTime, t_window_definition window);

    glm::mat4 constructPerspectiveProjectionMatrix(
            const Camera &camera, const Ellipsoid &ellipsoid, const t_window_definition &window);

    void updateTexturesWithData(const std::vector<TextureLoadResult> &results);

    bool getOrPrepareTexture(
            const std::shared_ptr<TileResources> &resources,
            const Tile &tile,
            TextureType textureType,
            std::shared_ptr<Texture> &texture);
};


#endif //EARTH_VISUALIZATION_TILEEARTHRENDERER_H
