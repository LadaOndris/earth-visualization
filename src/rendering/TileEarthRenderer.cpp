
#include "TileEarthRenderer.h"

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
    for (Tile &tile : tileContainer.getTiles()) {
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
            resources->setMeshBufferOffset(fullMeshForThisLevel.size());

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
    glGenBuffers(1, &VBO);

    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(t_vertex), &vertices.front(), GL_STATIC_DRAW);

    // Position
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *) 0);
    glEnableVertexAttribArray(0);
}

void TileEarthRenderer::prepareTexture(Texture &texture) {
    if (!texture.isLoaded()) {
        texture.load();
    }

    unsigned char *data = texture.getData();
    Resolution resolution = texture.getResolution();

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, resolution.getWidth(), resolution.getHeight(), 0, GL_RGB, GL_UNSIGNED_BYTE,
                 data);
    glGenerateMipmap(GL_TEXTURE_2D);
}

void TileEarthRenderer::render(float currentTime, t_window_definition window, RenderingOptions options) {
    shader.use();

    glGenTextures(1, &dayTextureId); // TODO: is this okay?

    // Set up model, view, and projection matrix
    glm::mat4 viewProjection = setupMatrices(currentTime, window);
    // Set ellipsoid parameters for the vertex shader
    shader.setVec3("ellipsoidRadiiSquared", ellipsoid.getRadii());
    shader.setVec3("ellipsoidOneOverRadiiSquared", ellipsoid.getOneOverRadiiSquared());

    auto tiles = tileContainer.getTiles();
    int skipTiles = 1024;
    int drawTiles = 1;
    int frustumCulledTiles = 0;

    for (Tile &tile: tiles) {
        skipTiles--;
        if (skipTiles < 0) {
            //continue;
        }
        if (drawTiles == 0) {
            //break;
        }
        // Frustum culling
        if (!tile.isInViewFrustum(viewProjection)) {
            frustumCulledTiles++;
            continue;
        }

        shader.setFloat("uTileLongitudeOffset", tile.getLongitude());
        shader.setFloat("uTileLatitudeOffset", tile.getLatitude());
        shader.setFloat("uTileLongitudeWidth", tile.getLongitudeWidth());
        shader.setFloat("uTileLatitudeWidth", tile.getLatitudeWidth());

        double screenSpaceWidth = window.width;
        double distanceToCamera = glm::length(camera.getPosition() - tile.getGeocentricPosition());
        double fov = camera.getFov();


        std::shared_ptr<TileResources> resources = tile.getResources(screenSpaceWidth, distanceToCamera, fov);
        Mesh_t mesh = resources->getMesh();

        // Load resources if needed
//        Texture &dayTexture = resources->getDayTexture();
//        prepareTexture(dayTexture);

        // Bind texture
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, dayTextureId);

        // Set fragment shader variables: texture, lighting
        // Texture: offset

        // Set VAO: we need the correct buffer? Is there one or more?
        glBindVertexArray(resources->meshVAO);

        if (options.isWireframeEnabled) {
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        }
        else{
            glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
        }
        glDrawArrays(GL_TRIANGLES, 0, mesh.size());
        drawTiles--;
        // Draw mesh
    }

    std::cout << "Frustum culled tiles: " << frustumCulledTiles << "/" << tiles.size() << std::endl;
}

glm::mat4 TileEarthRenderer::setupMatrices(float currentTime, t_window_definition window) {
    glm::mat4 projectionMatrix;
    projectionMatrix = glm::perspective(glm::radians(camera.getFov()),
                                        (float) window.width / (float) window.height,
                                        0.001f, 500.0f);
    glm::mat4 modelMatrix = glm::mat4(1.0f);
    glm::vec3 ellipsoidPosition = glm::vec3(0.f, 0.f, 0.f);
    modelMatrix = glm::translate(modelMatrix, ellipsoidPosition);
    float angle = 20.0f * 0;
    modelMatrix = glm::rotate(modelMatrix, currentTime * glm::radians(angle),
                              glm::vec3(1.0f, 0.3f, 0.5f));
    glm::mat4 viewMatrix = camera.getViewMatrix();

    shader.setMat4("projection", projectionMatrix);
    shader.setMat4("view", viewMatrix);
    shader.setMat4("model", modelMatrix);

    glm::mat4 viewProjection = projectionMatrix * viewMatrix;
    return viewProjection;
}

void TileEarthRenderer::destroy() {
    // Release texturee
    glDeleteTextures(1, &dayTextureId);
    // Release buffers
    int numLevels = tileContainer.getNumLevels();
    for (int level = 0; level < numLevels; level++) {
        Tile &tile = tileContainer.getTiles()[0];
        auto resources = tile.getResourcesByLevel(level);
        glDeleteVertexArrays(1, &resources->meshVAO);
        glDeleteBuffers(1, &resources->meshVBO);
    }

    // TODO: unload textures
}


