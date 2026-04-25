
#include "TileEarthRenderer.h"
#include "MeshBuffer.h"
#include "RendererSubscriber.h"
#include "utils.h"

#include <unistd.h>
#include <algorithm>

TileEarthRenderer::TileEarthRenderer(TileContainer &tileContainer,
                                     Ellipsoid &ellipsoid,
                                     Camera &camera,
                                     LightSource &lightSource,
                                     AsyncTextureLoader &textureLoader,
                                     ResourceManager &resourceManager,
                                     Program &program)
        : _tileContainer(tileContainer),
          _ellipsoid(ellipsoid),
          _camera(camera),
          _lightSource(lightSource),
          _textureLoader(textureLoader),
          _resourceManager(resourceManager),
          _program(program) {
    for (Tile &tile: _tileContainer.getTiles()) {
        tile.updateGeocentricPosition(_ellipsoid);
    }
    _program.build();
    initVertexArraysForAllLevels(_tileContainer.getNumLevels());
}

TileEarthRenderer::~TileEarthRenderer() {
    _resourceManager.releaseAll();
}

/**
 * Creates a vertex buffer for each level of detail (LOD).
 *
 * These vertex buffers contain the full geometry of each level.
 *
 * @param numLevels The number of level of details.
 */
void TileEarthRenderer::initVertexArraysForAllLevels(int numLevels) {
    for (int level = 0; level < numLevels; level++) {
        // TODO: All tiles share the same mesh; use the first tile's mesh for this level.
        Mesh_t mesh = _tileContainer.getTiles()[0].getResourcesByLevel(level)->getMesh();
        auto buffer = std::make_shared<MeshBuffer>(convertToVertices(mesh));

        for (Tile &tile: _tileContainer.getTiles())
            tile.getResourcesByLevel(level)->setMeshBuffer(buffer);
    }
}

bool TileEarthRenderer::prepareTexture(const std::shared_ptr<Texture> &texture) {
    if (texture->isPreparedInGlContext()) {
        // The texture is ready to use in OpenGL
        // Notify the resource manager about the current usage of textures
        _resourceManager.noteUsage(texture);
        return true;
    } else {
        // Check if a request has been made for this texture
        auto it = _requestMap.find(texture->getPath());
        if (it == _requestMap.end()) {
            // The texture hasn't been loaded from disk
            TextureLoadRequest request = {
                    .path = texture->getPath()
            };
            _textureLoader.request(request);
            // Register a request into a data structure
            // so that it can be connected to a TextureLoadResult by the path
            _requestMap[texture->getPath()] = texture;
        }
        return false;

    }
}

void TileEarthRenderer::updateTexturesWithData(const std::vector<TextureLoadResult> &results) {
    for (const TextureLoadResult &result: results) {
        // Get the instance of the texture from the HashMap
        auto it = _requestMap.find(result.path);
        if (it != _requestMap.end()) {
            std::shared_ptr<Texture> texture = it->second;

            // Copy the data from the TextureLoadResult to the texture instance.
            texture->setData(result.data);
            texture->setChannels(result.channels);

            // Remove the registration from the HashMap
            _requestMap.erase(it);

            assert(result.width == texture->getResolution().getWidth());
            assert(result.height == texture->getResolution().getHeight());

            // Now, the texture is loaded and can be prepared for OpenGL
            _resourceManager.addTextureIntoContext(texture);
        }
    }
}

bool TileEarthRenderer::getOrPrepareTexture(
        const std::shared_ptr<TileResources> &resources,
        const Tile &tile,
        const TextureType textureType,
        std::shared_ptr<Texture> &texture) {

    // Set up the neccessary texture
    texture = resources->getTexture(textureType);

    // Request and prepare the texture
    bool textureReady = prepareTexture(texture);
    if (!textureReady) {
        // Search for coarser textures
        textureReady = resources->getCoarserTexture(texture, textureType);

        // And, possibly, search for fine-grained textures
        if (!textureReady) {
            textureReady = resources->getFinerTexture(tile, texture, textureType);
        }
    }
    return textureReady;
}

void TileEarthRenderer::render(float currentTime, t_window_definition window, RenderingOptions options) {
    auto newlyLoadedTexturesData = _textureLoader.getResults();
    updateTexturesWithData(newlyLoadedTexturesData);

    _program.use();
    _program.setInt("dayTextureSampler", 0); // Texture Unit 0
    _program.setInt("nightTextureSampler", 1); // Texture Unit 1
    _program.setInt("heightMapSampler", 2); // Texture Unit 2
    _program.setBool("useDayTexture", options.isTextureEnabled);
    _program.setBool("isNightEnabled", options.isNightEnabled);
    _program.setBool("displayGrid", options.isGridEnabled);
    _program.setBool("isTerrainEnabled", options.isTerrainEnabled);
    _program.setBool("isTerrainShadingEnabled", options.isTerrainShadingEnabled);

    _program.setFloat("gridResolution", 0.05);
    _program.setFloat("gridLineWidth", 2);

    // Day/night blending
    float blendDuration = 0.3f;
    _program.setFloat("blendDuration", blendDuration);
    _program.setFloat("blendDurationScale", 1 / (2 * blendDuration));

    // Height map settings
    double ellipsoidScaleFactor = _ellipsoid.getRealityScaleFactor();
    double displacementFactor = 25. / ellipsoidScaleFactor * options.heightFactor;
    _program.setFloat("heightDisplacementFactor", static_cast<float>(displacementFactor));
    _program.setInt("heightScale", options.heightFactor);

    // Set up model, view, and projection matrix
    Frustum frustum = setupMatrices(currentTime, window);
    // Set ellipsoid parameters for the vertex program
    _program.setVec3("ellipsoidRadiiSquared", _ellipsoid.getRadiiSquared());
    _program.setVec3("ellipsoidOneOverRadiiSquared", _ellipsoid.getOneOverRadiiSquared());
    _program.setVec3("lightPos", _lightSource.getLightPosition());

    auto tiles = _tileContainer.getTiles();
    auto cameraPosition = _camera.getPosition();

    RenderingStatistics renderingStats;
    renderingStats.numTiles = tiles.size();

    double screenSpaceWidth = window.width;
    double minLatitude = std::numeric_limits<double>::infinity();
    double minLongitude = std::numeric_limits<double>::infinity();
    double maxLatitude = -std::numeric_limits<double>::infinity();
    double maxLongitude = -std::numeric_limits<double>::infinity();

    std::vector<Tile> renderedTiles;

    for (Tile &tile: tiles) {
        if (options.isCullingEnabled) {
            // Frustum culling
            if (!tile.isInViewFrustum(frustum)) {
                renderingStats.frustumCulledTiles++;
                continue;
            }
            // Backface culling
            if (!tile.isFacingCamera(cameraPosition)) {
                renderingStats.backfacedCulledTiles++;
                continue;
            }
        }
        renderedTiles.push_back(tile);
        minLongitude = std::min(tile.getLongitude(), minLongitude);
        minLatitude = std::min(tile.getLatitude(), minLatitude);
        maxLongitude = std::max(tile.getLongitude() + tile.getLongitudeWidth(), maxLongitude);
        maxLatitude = std::max(tile.getLatitude() + tile.getLatitudeWidth(), maxLatitude);

        _program.setFloat("uTileLongitudeOffset", tile.getLongitude());
        _program.setFloat("uTileLatitudeOffset", tile.getLatitude());
        _program.setFloat("uTileLongitudeWidth", tile.getLongitudeWidth());
        _program.setFloat("uTileLatitudeWidth", tile.getLatitudeWidth());

        double distanceToCamera = glm::length(_camera.getPosition() - tile.getGeocentricPosition());
        std::shared_ptr<TileResources> resources = tile.getResources(
                screenSpaceWidth, distanceToCamera, _camera);
        Mesh_t mesh = resources->getMesh();

        std::shared_ptr<Texture> dayTexture;
        std::shared_ptr<Texture> nightTexture;
        std::shared_ptr<Texture> heightMap;
        bool dayTextureReady = getOrPrepareTexture(resources, tile, TextureType::Day, dayTexture);
        bool nightTextureReady = getOrPrepareTexture(resources, tile, TextureType::Night, nightTexture);
        bool heightMapReady = getOrPrepareTexture(resources, tile, TextureType::HeightMap, heightMap);

        // Draw only if the necessary resources are ready
        if (dayTextureReady && nightTextureReady && heightMapReady) {
            // Set up day texture
            _program.setVec2("dayTextureGeodeticOffset", utils::convertToRads(dayTexture->getGeodeticOffset()));
            _program.setVec2("dayTextureGridSize", dayTexture->getTextureGridSize());
            glBindTextureUnit(0, dayTexture->getTextureId());

            // Set up night texture
            _program.setVec2("nightTextureGeodeticOffset", utils::convertToRads(nightTexture->getGeodeticOffset()));
            _program.setVec2("nightTextureGridSize", nightTexture->getTextureGridSize());
            glBindTextureUnit(1, nightTexture->getTextureId());

            // Set up height map
            _program.setVec2("heightMapGeodeticOffset", utils::convertToRads(heightMap->getGeodeticOffset()));
            _program.setVec2("heightMapGridSize", heightMap->getTextureGridSize());
            glBindTextureUnit(2, heightMap->getTextureId());

            glBindVertexArray(resources->vao());

            if (options.isWireframeEnabled) {
                glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            } else {
                glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            }
            glDrawArrays(GL_PATCHES, 0, mesh.size());
        }
    }

    // TODO: refactor: extract method
    auto geodeticCameraPosition = _ellipsoid.convertGeocentricToGeodetic(_camera.getPosition());
    auto surfacePoint = _ellipsoid.projectGeocentricPointOntoSurface(_camera.getPosition());
    auto distanceFromSurface = glm::length(_camera.getPosition() - surfacePoint);
    auto realityScaleFactor = _ellipsoid.getRealityScaleFactor();
    geodeticCameraPosition[2] = realityScaleFactor * distanceFromSurface;
    geodeticCameraPosition[1] *= -1; // Invert latitude (application uses a reversed latitude)

    renderingStats.loadedTextures = _resourceManager.getNumLoadedTextures();
    renderingStats.cameraPosition = geodeticCameraPosition;
    renderingStats.renderedLatitudeRange = glm::vec2(minLatitude, maxLatitude);
    renderingStats.renderedLongitudeRange = glm::vec2(minLongitude, maxLongitude);

    for (auto &subscriber: _subscribers) {
        subscriber->notify(renderingStats);
    }
}

glm::mat4 TileEarthRenderer::constructPerspectiveProjectionMatrix(
        const Camera &camera, const Ellipsoid &ellipsoid, const t_window_definition &window) {
    // Near and far plane has to be determined from the distance to Earth
    auto closestPointOnSurface = ellipsoid.projectGeocentricPointOntoSurface(camera.getPosition());
    auto distanceToSurface = glm::length(camera.getPosition() - closestPointOnSurface);
    auto distanceToEllipsoidsCenter = glm::length(camera.getPosition() - ellipsoid.getGeocentricPosition());

    // The near plane is set in the middle of the camera position and the surface
    auto nearPlane = distanceToSurface * 0.5f;
    auto farPlane = distanceToEllipsoidsCenter;

    glm::mat4 projectionMatrix;
    projectionMatrix = glm::perspective(glm::radians(camera.getFov()),
                                        static_cast<float>(window.width) / static_cast<float>(window.height),
                                        nearPlane, farPlane);
    return projectionMatrix;
}


Frustum TileEarthRenderer::setupMatrices(float currentTime, t_window_definition window) {
    glm::mat4 projectionMatrix = constructPerspectiveProjectionMatrix(_camera, _ellipsoid, window);
    glm::mat4 viewMatrix = _camera.getViewMatrix();

    // Do not rotate the model matrix to represent the Earth's inclination.
    // The inclination will be simulated using the position of the Sun
    glm::mat4 modelMatrix = glm::mat4(1.0f);
    //float inclinationAngle = glm::radians(23.5f); // Convert degrees to radians
    //modelMatrix = glm::rotate(modelMatrix, inclinationAngle, glm::vec3(1.0f, 0.0f, 0.0f));

    _program.setMat4("projection", projectionMatrix);
    _program.setMat4("view", viewMatrix);
    _program.setMat4("model", modelMatrix);

    return Frustum(viewMatrix, projectionMatrix);
}


void TileEarthRenderer::addSubscriber(const std::shared_ptr<RendererSubscriber> &subscriber) {
    _subscribers.push_back(subscriber);
}

