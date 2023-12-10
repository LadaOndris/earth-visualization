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
#include <glm/vec3.hpp>
#include <glm/detail/type_vec2.hpp>

struct Character {
    glm::vec2 size;       // Size of glyph
    glm::vec2 bearing;    // Offset from baseline to left/top of glyph
    long advanceX;
    long advanceY;
    float textureOffsetX; // x offset of glyph in texture coordinates
};


//struct character_info {
//    float ax; // advance.x
//    float ay; // advance.y
//
//    float bw; // bitmap.width;
//    float bh; // bitmap.rows;
//
//    float bl; // bitmap_left;
//    float bt; // bitmap_top;
//
//    float tx; // x offset of glyph in texture coordinates
//};

class CityNamesRenderer : public Renderer {
private:
    std::map<char, Character> characters;
    GLuint textureId;
    float atlasWidth, atlasHeight;
    Program &program;
    unsigned int VAO, VBO;

    bool prepareTextureAtlas();

    bool prepareBuffers();

    void renderText(const char *text, float x, float y, float sx, float sy, glm::vec3 color);

public:
    explicit CityNamesRenderer(Program &program);

    bool initialize() override;

    void destroy() override;

    void render(float currentTime, t_window_definition window, RenderingOptions options) override;

};


#endif //EARTH_VISUALIZATION_CITYNAMESRENDERER_H
