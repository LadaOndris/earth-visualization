//
// Created by lada on 12/10/23.
//

#ifndef EARTH_VISUALIZATION_CITYNAMESRENDERER_H
#define EARTH_VISUALIZATION_CITYNAMESRENDERER_H

#include <map>
#include "Renderer.h"
#include "RenderingOptions.h"
#include "../include/glad/glad.h"
#include "program.h"
#include "../cameras/Camera.h"
#include "../ellipsoid.h"
#include "../WorldCitiesReader.h"
#include "RendererSubscriber.h"
#include "../Frustum.h"
#include <glm/vec3.hpp>
#include <glm/detail/type_vec2.hpp>

struct Character {
    glm::vec2 size;       // Size of glyph
    glm::vec2 bearing;    // Offset from baseline to left/top of glyph
    long advanceX;
    long advanceY;
    float textureOffsetX; // x offset of glyph in texture coordinates
};

struct VertexData {
    GLfloat x;
    GLfloat y;
    GLfloat s;
    GLfloat t;
};

struct TextInstanceData {
    float latitude;
    float longitude;
};


class CityNamesRenderer : public Renderer, public RendererSubscriber {
private:
    std::map<char, Character> characters;
    GLuint textureId;
    float atlasWidth, atlasHeight;
    Program &program;
    unsigned int VAO, VBO, instanceVBO;
    Camera &camera;
    Ellipsoid &ellipsoid;
    std::vector<City> worldCities;
    RenderingStatistics rendereringStats;

    bool prepareTextureAtlas();

    bool prepareBuffers();

    void renderText(const City &text, float sx, float sy, glm::vec3 color);

    void renderTexts(const std::vector<City> &texts, float sx, float sy, glm::vec3 color);

    void renderTextsInstanced(const std::vector<City> &texts, float sx, float sy, glm::vec3 color);

    int setVertexDataForText(const City &text, float sx, float sy, VertexData *vertexData);

    glm::mat4 constructPerspectiveProjectionMatrix(
            const Camera &camera, const Ellipsoid &ellipsoid, const t_window_definition &window);

    bool isRenderedAreaTooBig() const;

    void retrieveDataToBeRendered(const Frustum &frustum, std::vector<City> &out) const;

public:
    explicit CityNamesRenderer(Program &program, Camera &camera, Ellipsoid &ellipsoid);

    bool initialize() override;

    void destroy() override;

    void render(float currentTime, t_window_definition window, RenderingOptions options) override;

    void notify(RenderingStatistics renderingStatistics) override;
};


#endif //EARTH_VISUALIZATION_CITYNAMESRENDERER_H
