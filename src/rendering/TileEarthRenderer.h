//
// Created by lada on 10/19/23.
//

#ifndef EARTH_VISUALIZATION_TILEEARTHRENDERER_H
#define EARTH_VISUALIZATION_TILEEARTHRENDERER_H


#include <glm/vec3.hpp>
#include <unordered_map>
#include "Renderer.h"
#include "../cameras/Camera.h"
#include "../ellipsoid.h"
#include "../tiling/TileContainer.h"
#include "program.h"
#include "../vertex.h"
#include "RendererSubscriber.h"
#include "../resources/ResourceFetcher.h"
#include "../resources/ResourceManager.h"
#include "../simulation/LightSource.h"

class TileEarthRenderer : public Renderer {
private:
    float TO_RADS_COEFF = static_cast<float>(M_PI / 180.0);
    TileContainer &tileContainer;
    Camera &camera;
    Ellipsoid &ellipsoid;
    ResourceFetcher &resourceFetcher;
    ResourceManager &resourceManager;
    const LightSource &lightSource;
    Program &program;
    std::vector<std::shared_ptr<RendererSubscriber>> subscribers;
    std::unordered_map<std::string, std::shared_ptr<Texture>> requestMap;


    void initVertexArraysForAllLevels(int numLevels);

    void setupVertexArray(std::vector<t_vertex> vertices,
                          unsigned int &VAO, unsigned int &VBO);

    bool prepareTexture(const std::shared_ptr<Texture>& texture);

    glm::mat4 setupMatrices(float currentTime, t_window_definition window);

    glm::mat4 constructPerspectiveProjectionMatrix(
            const Camera &camera, const Ellipsoid &ellipsoid, const t_window_definition &window);

    void updateTexturesWithData(const std::vector<TextureLoadResult> &results);

    bool getOrPrepareTexture(
            const std::shared_ptr<TileResources> &resources,
            const Tile &tile,
            TextureType textureType,
            std::shared_ptr<Texture> &texture);
public:
    explicit TileEarthRenderer(TileContainer &tileContainer,
                               Ellipsoid &ellipsoid,
                               Camera &camera,
                               LightSource &lightSource,
                               ResourceFetcher &resourceFetcher,
                               ResourceManager &resourceManager,
                               Program &program)
            : tileContainer(tileContainer), camera(camera), ellipsoid(ellipsoid),
              lightSource(lightSource), resourceFetcher(resourceFetcher),
              resourceManager(resourceManager),
              program(program) {
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
