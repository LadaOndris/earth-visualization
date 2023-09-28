//
// Created by lada on 9/26/23.
//

#include "EarthRenderer.h"
#include <vector>
#include <glm/ext/matrix_float4x4.hpp>
#include "../vertex.h"
#include "shader.h"
#include <glm/vec3.hpp> // glm::vec3
#include <glm/vec4.hpp> // glm::vec4
#include <glm/mat4x4.hpp> // glm::mat4
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "../include/stb_image.h"
#include "../tesselation/SubdivisionSphereTesselator.h"

void EarthRenderer::constructVertices(SphereTesselator &tesselator) {
    std::cout << "Generating triangles..." << std::endl;

    auto verticesVecs = tesselator.tessellate(6);
    auto projected_vertices = ellipsoid.projectPointsOntoSurface(std::move(verticesVecs));

    vertices = convertToVertices(projected_vertices);

    std::cout << "Generated " + std::to_string(vertices.size() / 3) + " triangles." << std::endl;

}

std::vector<t_vertex> EarthRenderer::convertToVertices(const std::vector<glm::vec3> &projectedVertices) {
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

void EarthRenderer::setupVertexArrays() {
    glGenBuffers(1, &VBO);

    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(t_vertex), &vertices.front(), GL_STATIC_DRAW);

    // Position
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *) 0);
    glEnableVertexAttribArray(0);
}

void EarthRenderer::loadTextures() {
    glGenTextures(1, &dayTexture);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, dayTexture);
    loadTexture(dayTexture, "textures/2_no_clouds_16k.jpg");

    glGenTextures(1, &nightTexture);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, nightTexture);
    loadTexture(nightTexture, "textures/5_night_16k.jpg");
}

void EarthRenderer::loadTexture(unsigned int &textureID, const std::string &texturePath) {
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    int width, height, nrChannels;
    stbi_set_flip_vertically_on_load(true);
    unsigned char *data = stbi_load(texturePath.c_str(), &width, &height, &nrChannels, 0);
    if (data) {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    } else {
        std::cout << "Failed to load texture" << std::endl;
    }
    stbi_image_free(data);
}

void EarthRenderer::render(float currentTime, t_window_definition window) {
    shader.use();

    glm::mat4 projectionMatrix;
    projectionMatrix = glm::perspective(glm::radians(camera.getFov()),
                                        (float) window.width / (float) window.height,
                                        0.1f, 100.0f);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, dayTexture);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, nightTexture);

    // Texture settings
    float blendDuration = 0.3f;
    shader.setInt("dayTexture", 0);
    shader.setInt("nightTexture", 1);
    shader.setFloat("blendDuration", blendDuration);
    shader.setFloat("blendDurationScale", 1 / (2 * blendDuration));

    // Lighting settings
    shader.setVec3("objectColor", glm::vec3(1.0f, 0.5f, 0.31f));
    shader.setVec3("lightColor", glm::vec3(1.0f, 1.0f, 1.0f));
    shader.setVec3("lightPos", lightPosition);
    shader.setVec3("oneOverRadiiSquared", ellipsoid.getOneOverRadiiSquared());

    // note: currently we set the projection matrix each frame, but since the projection matrix rarely changes
    // it's often best practice to set it outside the main loop only once.
    shader.setMat4("projection", projectionMatrix);
    shader.setMat4("view", camera.getViewMatrix());
    //print_mat4(camera.getViewMatrix());

    glBindVertexArray(VAO);
    glm::mat4 modelMatrix = glm::mat4(1.0f);
    glm::vec3 ellipsoidPosition = glm::vec3(0.f, 0.f, 0.f);
    modelMatrix = glm::translate(modelMatrix, ellipsoidPosition);
    float angle = 20.0f * 0;
    modelMatrix = glm::rotate(modelMatrix, currentTime * glm::radians(angle),
                              glm::vec3(1.0f, 0.3f, 0.5f));
    shader.setMat4("model", modelMatrix);

    glDrawArrays(GL_TRIANGLES, 0, vertices.size());
}