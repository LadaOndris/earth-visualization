
#include "TileEarthRenderer.h"
#include "RendererSubscriber.h"
#include <unistd.h>

bool TileEarthRenderer::initialize() {
    // Configure tiles to use the current ellipsoid
    for (Tile &tile: tileContainer.getTiles()) {
        tile.updateGeocentricPosition(ellipsoid);
    }

    bool isShaderProgramBuilt = program.build();
    if (!isShaderProgramBuilt) {
        return false;
    }
    int numLevels = tileContainer.getNumLevels();
    initVertexArraysForAllLevels(numLevels);

    return true;
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
        Mesh_t fullMeshForThisLevel;
        // Merge all tile meshes of this level together.
        for (Tile &tile: tileContainer.getTiles()) {
            auto resources = tile.getResourcesByLevel(level);

            Mesh_t mesh = resources->getMesh();
            fullMeshForThisLevel.insert(fullMeshForThisLevel.end(), mesh.begin(), mesh.end());
            break; // TODO: remove cycle. All tiles share the same mesh.
        }

        std::vector<t_vertex> verticesForThisLevel = convertToVertices(fullMeshForThisLevel);

        unsigned int VAO, VBO;
        setupVertexArray(verticesForThisLevel, VAO, VBO);

        for (Tile &tile: tileContainer.getTiles()) {
            auto resources = tile.getResourcesByLevel(level);
            resources->meshVAO = VAO;
            resources->meshVBO = VBO;
        }
    }
}

void TileEarthRenderer::setupVertexArray(std::vector<t_vertex> vertices,
                                         unsigned int &VAO, unsigned int &VBO) {
    glCreateBuffers(1, &VBO);

    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glNamedBufferData(VBO, vertices.size() * sizeof(t_vertex), &vertices.front(), GL_STATIC_DRAW);

    // Position
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *) 0);
    glEnableVertexAttribArray(0);
}

bool TileEarthRenderer::prepareTexture(const std::shared_ptr<Texture> &texture) {
    if (texture->isPreparedInGlContext()) {
        // The texture is ready to use in OpenGL
        // Notify the resource manager about the current usage of textures
        resourceManager.noteUsage(texture);
        return true;
    } else {
        // Check if a request has been made for this texture
        auto it = requestMap.find(texture->getPath());
        if (it == requestMap.end()) {
            // The texture hasn't been loaded from disk
            TextureLoadRequest request = {
                    .path = texture->getPath()
            };
            resourceFetcher.request(request);
            // Register a request into a data structure
            // so that it can be connected to a TextureLoadResult by the path
            requestMap[texture->getPath()] = texture;
        }
        return false;

    }
}

void TileEarthRenderer::updateTexturesWithData(const std::vector<TextureLoadResult> &results) {
    for (const TextureLoadResult &result: results) {
        // Get the instance of the texture from the HashMap
        auto it = requestMap.find(result.path);
        if (it != requestMap.end()) {
            std::shared_ptr<Texture> texture = it->second;

            // Copy the data from the TextureLoadResult to the texture instance.
            texture->setData(result.data);
            texture->setChannels(result.channels);

            // Remove the registration from the HashMap
            requestMap.erase(it);

            assert(result.width == texture->getResolution().getWidth());
            assert(result.height == texture->getResolution().getHeight());

            // Now, the texture is loaded and can be prepared for OpenGL
            resourceManager.addTextureIntoContext(texture);
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
    auto newlyLoadedTexturesData = resourceFetcher.retrieveLoadedResources();
    updateTexturesWithData(newlyLoadedTexturesData);

    program.use();
    program.setInt("dayTextureSampler", 0); // Texture Unit 0
    program.setInt("nightTextureSampler", 1); // Texture Unit 1
    program.setInt("heightMapSampler", 2); // Texture Unit 2
    program.setBool("useDayTexture", options.isTextureEnabled);
    program.setBool("isNightEnabled", options.isNightEnabled);
    program.setBool("displayGrid", options.isGridEnabled);
    program.setBool("isTerrainEnabled", options.isTerrainEnabled);
    program.setBool("isTerrainShadingEnabled", options.isTerrainShadingEnabled);

    program.setFloat("gridResolution", 0.05);
    program.setFloat("gridLineWidth", 2);

    // Day/night blending
    float blendDuration = 0.3f;
    program.setFloat("blendDuration", blendDuration);
    program.setFloat("blendDurationScale", 1 / (2 * blendDuration));

    // Height map settings
    double ellipsoidScaleFactor = ellipsoid.getRealityScaleFactor();
    double displacementFactor = 25. / ellipsoidScaleFactor * options.heightFactor;
    program.setFloat("heightDisplacementFactor", static_cast<float>(displacementFactor));
    program.setInt("heightScale", options.heightFactor);

    // Set up model, view, and projection matrix
    glm::mat4 viewProjection = setupMatrices(currentTime, window);
    // Set ellipsoid parameters for the vertex program
    program.setVec3("ellipsoidRadiiSquared", ellipsoid.getRadiiSquared());
    program.setVec3("ellipsoidOneOverRadiiSquared", ellipsoid.getOneOverRadiiSquared());
    program.setVec3("lightPos", lightSource.getLightPosition());

    auto tiles = tileContainer.getTiles();
    auto cameraPosition = camera.getPosition();

    RenderingStatistics renderingStats;
    renderingStats.numTiles = tiles.size();

    double screenSpaceWidth = window.width;

    for (Tile &tile: tiles) {
        if (options.isCullingEnabled) {
            // Frustum culling
            if (!tile.isInViewFrustum(viewProjection)) {
                renderingStats.frustumCulledTiles++;
                continue;
            }
            // Backface culling
            if (!tile.isFacingCamera(cameraPosition)) {
                renderingStats.backfacedCulledTiles++;
                continue;
            }
        }

        program.setFloat("uTileLongitudeOffset", tile.getLongitude());
        program.setFloat("uTileLatitudeOffset", tile.getLatitude());
        program.setFloat("uTileLongitudeWidth", tile.getLongitudeWidth());
        program.setFloat("uTileLatitudeWidth", tile.getLatitudeWidth());

        double distanceToCamera = glm::length(camera.getPosition() - tile.getGeocentricPosition());
        std::shared_ptr<TileResources> resources = tile.getResources(
                screenSpaceWidth, distanceToCamera, camera);
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
            program.setVec2("dayTextureGeodeticOffset", dayTexture->getGeodeticOffset() * TO_RADS_COEFF);
            program.setVec2("dayTextureGridSize", dayTexture->getTextureGridSize());
            glBindTextureUnit(0, dayTexture->getTextureId());

            // Set up night texture
            program.setVec2("nightTextureGeodeticOffset", nightTexture->getGeodeticOffset() * TO_RADS_COEFF);
            program.setVec2("nightTextureGridSize", nightTexture->getTextureGridSize());
            glBindTextureUnit(1, nightTexture->getTextureId());

            // Set up height map
            program.setVec2("heightMapGeodeticOffset", heightMap->getGeodeticOffset() * TO_RADS_COEFF);
            program.setVec2("heightMapGridSize", heightMap->getTextureGridSize());
            glBindTextureUnit(2, heightMap->getTextureId());

            // Set VAO: we need the correct buffer? Is there one or more?
            glBindVertexArray(resources->meshVAO);

            if (options.isWireframeEnabled) {
                glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            } else {
                glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            }
            glDrawArrays(GL_PATCHES, 0, mesh.size());
        }
    }

    // TODO: refactor: extract method
    auto geodeticCameraPosition = ellipsoid.convertGeocentricToGeodetic(camera.getPosition());
    auto surfacePoint = ellipsoid.projectGeocentricPointOntoSurface(camera.getPosition());
    auto distanceFromSurface = glm::length(camera.getPosition() - surfacePoint);
    auto realityScaleFactor = ellipsoid.getRealityScaleFactor();
    geodeticCameraPosition[2] = realityScaleFactor * distanceFromSurface;

    renderingStats.loadedTextures = resourceManager.getNumLoadedTextures();
    renderingStats.cameraPosition = geodeticCameraPosition;

    for (auto &subscriber: subscribers) {
        subscriber->notify(renderingStats);
    }
}

glm::mat4 TileEarthRenderer::setupMatrices(float currentTime, t_window_definition window) {
    // Near and far plane has to be determined from the distance to Earth
    auto closestPointOnSurface = ellipsoid.projectGeocentricPointOntoSurface(camera.getPosition());
    auto distanceToSurface = glm::length(camera.getPosition() - closestPointOnSurface);
    auto distanceToEllipsoidsCenter = glm::length(camera.getPosition() - ellipsoid.getGeocentricPosition());

    // The near plane is set in the middle of the camera position and the surface
    float nearPlane = static_cast<float>(distanceToSurface * 0.5);
    float farPlane = distanceToEllipsoidsCenter;

    glm::mat4 projectionMatrix;
    projectionMatrix = glm::perspective(glm::radians(camera.getFov()),
                                        (float) window.width / (float) window.height,
                                        nearPlane, farPlane);
    glm::mat4 viewMatrix = camera.getViewMatrix();

    // Do not rotate the model matrix to represent the Earth's inclination.
    // The inclination will be simulated using the position of the Sun
    glm::mat4 modelMatrix = glm::mat4(1.0f);
    //float inclinationAngle = glm::radians(23.5f); // Convert degrees to radians
    //modelMatrix = glm::rotate(modelMatrix, inclinationAngle, glm::vec3(1.0f, 0.0f, 0.0f));

    program.setMat4("projection", projectionMatrix);
    program.setMat4("view", viewMatrix);
    program.setMat4("model", modelMatrix);

    glm::mat4 viewProjection = projectionMatrix * viewMatrix;
    return viewProjection;
}

void TileEarthRenderer::destroy() {
    // Release textures
    resourceManager.releaseAll();

    // Release buffers
    int numLevels = tileContainer.getNumLevels();
    for (int level = 0; level < numLevels; level++) {
        Tile &tile = tileContainer.getTiles()[0];
        auto resources = tile.getResourcesByLevel(level);
        glDeleteVertexArrays(1, &resources->meshVAO);
        glDeleteBuffers(1, &resources->meshVBO);
    }
}

void TileEarthRenderer::addSubscriber(const std::shared_ptr<RendererSubscriber> &subscriber) {
    subscribers.push_back(subscriber);
}

