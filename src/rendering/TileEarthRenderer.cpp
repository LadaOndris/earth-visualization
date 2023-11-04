
#include "TileEarthRenderer.h"
#include "RendererSubscriber.h"
#include <unistd.h>

std::vector<t_vertex> convertToVertices(const std::vector<glm::vec3> &projectedVertices) {
    std::vector<t_vertex> convertedVertices;
    for (const auto &vec3: projectedVertices) {
        t_vertex vertex;
        vertex.x = vec3.x;
        vertex.y = vec3.y;
        vertex.z = vec3.z;

        convertedVertices.push_back(vertex);
    }
    return convertedVertices;
}

bool TileEarthRenderer::initialize() {
    // Configure tiles to use the current ellipsoid
    for (Tile &tile: tileContainer.getTiles()) {
        tile.updateGeocentricPosition(ellipsoid);
    }

    bool isShaderProgramBuilt = shader.build();
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

            // Remove the registration from the HashMap
            requestMap.erase(it);

            assert(result.width == texture->getResolution().getWidth());
            assert(result.height == texture->getResolution().getHeight());

            // Now, the texture is loaded and can be prepared for OpenGL
            resourceManager.addTextureIntoContext(texture);
        }
    }
}

void TileEarthRenderer::render(float currentTime, t_window_definition window, RenderingOptions options) {
    auto newlyLoadedTexturesData = resourceFetcher.retrieveLoadedResources();
    updateTexturesWithData(newlyLoadedTexturesData);

    shader.use();
    shader.setInt("dayTextureSampler", 0); // Texture Unit 0
    shader.setBool("useDayTexture", options.isTextureEnabled);
    shader.setBool("displayGrid", options.isGridEnabled);

    shader.setFloat("gridResolution", 0.05);
    shader.setFloat("gridLineWidth", 2);

    // Set up model, view, and projection matrix
    glm::mat4 viewProjection = setupMatrices(currentTime, window);
    // Set ellipsoid parameters for the vertex shader
    shader.setVec3("ellipsoidRadiiSquared", ellipsoid.getRadiiSquared());
    shader.setVec3("ellipsoidOneOverRadiiSquared", ellipsoid.getOneOverRadiiSquared());
    shader.setVec3("lightPos", lightPosition);

    auto tiles = tileContainer.getTiles();
    auto cameraPosition = camera.getPosition();

    RenderingStatistics renderingStats;
    renderingStats.numTiles = tiles.size();

    double screenSpaceWidth = window.width;
    double fov = camera.getFov();

    for (Tile &tile: tiles) {
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

        shader.setFloat("uTileLongitudeOffset", tile.getLongitude());
        shader.setFloat("uTileLatitudeOffset", tile.getLatitude());
        shader.setFloat("uTileLongitudeWidth", tile.getLongitudeWidth());
        shader.setFloat("uTileLatitudeWidth", tile.getLatitudeWidth());

        double distanceToCamera = glm::length(camera.getPosition() - tile.getGeocentricPosition());
        std::shared_ptr<TileResources> resources = tile.getResources(screenSpaceWidth, distanceToCamera, fov);
        Mesh_t mesh = resources->getMesh();

        // Set up the neccessary texture
        auto dayTexture = resources->getDayTexture();

        // Draw only if the necessary resources are ready
        if (prepareTexture(dayTexture)) {
            shader.setVec2("textureGeodeticOffset", dayTexture->getGeodeticOffset() * TO_RADS_COEFF);
            shader.setVec2("textureGridSize", dayTexture->getTextureGridSize());

            glBindTextureUnit(0, dayTexture->getTextureId());

            // Set VAO: we need the correct buffer? Is there one or more?
            glBindVertexArray(resources->meshVAO);

            if (options.isWireframeEnabled) {
                glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            } else {
                glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            }
            glDrawArrays(GL_TRIANGLES, 0, mesh.size());
        }
    }
    auto geodeticCameraPosition = ellipsoid.convertGeocentricToGeodetic(camera.getPosition());

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
    glm::mat4 modelMatrix = glm::mat4(1.0f);
    glm::vec3 ellipsoidPosition = glm::vec3(0.f, 0.f, 0.f);
    //modelMatrix = glm::translate(modelMatrix, ellipsoidPosition);
    //float angle = 20.0f * 0;
    //modelMatrix = glm::rotate(glm::mat4(1.0f), glm::radians(180.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    glm::mat4 viewMatrix = camera.getViewMatrix();

    shader.setMat4("projection", projectionMatrix);
    shader.setMat4("view", viewMatrix);
    shader.setMat4("model", modelMatrix);

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

