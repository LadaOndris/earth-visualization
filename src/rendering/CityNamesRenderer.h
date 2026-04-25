
#pragma once

#include "Renderer.h"
#include "RenderingOptions.h"
#include "glad/glad.h"
#include "program.h"
#include "cameras/Camera.h"
#include "ellipsoid.h"
#include "WorldCitiesReader.h"
#include "RendererSubscriber.h"
#include "Frustum.h"

#include <glm/vec3.hpp>
#include <glm/detail/type_vec2.hpp>

#include <map>

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
public:
    explicit CityNamesRenderer(Program &program, Camera &camera, Ellipsoid &ellipsoid);

    bool initialize() override;

    void destroy() override;

    void render(float currentTime, t_window_definition window, RenderingOptions options) override;

    void notify(RenderingStatistics renderingStatistics) override;

private:
    std::map<char, Character> _characters;
    GLuint _textureId;
    float _atlasWidth;
    float _atlasHeight;
    Program &_program;
    unsigned int _VAO, _VBO, _instanceVBO;
    Camera &_camera;
    Ellipsoid &_ellipsoid;
    std::vector<City> _worldCities;
    RenderingStatistics _renderingStats;

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

};
