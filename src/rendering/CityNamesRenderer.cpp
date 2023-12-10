//
// Created by lada on 12/10/23.
//

#include <algorithm>
#include <cstring>
#include <cstdio>
#include "CityNamesRenderer.h"
#include <ft2build.h>
#include FT_FREETYPE_H

bool CityNamesRenderer::prepareTextureAtlas() {
    FT_Library library;   /* handle to library     */
    FT_Face face;      /* handle to face object */

    // Initialize library
    auto error = FT_Init_FreeType(&library);
    if (error) {
        fprintf(stderr, "Freetype library initialization failed.");
        return false;
    }
    // Load font
    error = FT_New_Face(library,
                        "data/fonts/verdana.ttf",
                        0,
                        &face);
    if (error == FT_Err_Unknown_File_Format) {
        fprintf(stderr, "ERROR::FREETYPE: The font file could be opened and read, but it appears"
                        "that its font format is unsupported.");
        return false;
    } else if (error) {
        fprintf(stderr, "ERROR::FREETYPE: The font file could not"
                        "be opened or read, or that it is broken.");
        return false;
    }
    FT_Set_Pixel_Sizes(face, 0, 48);

    FT_GlyphSlot g = face->glyph;
    unsigned int w = 0;
    unsigned int h = 0;

    for (int i = 32; i < 128; i++) {
        if (FT_Load_Char(face, i, FT_LOAD_RENDER)) {
            fprintf(stderr, "Loading character %c failed!\n", i);
            continue;
        }

        w += g->bitmap.width;
        h = std::max(h, g->bitmap.rows);
    }

    /* you might as well save this value as it is needed later on */
    atlasWidth = static_cast<float>(w);
    atlasHeight = static_cast<float>(h);

    // Create texture atlas
    glActiveTexture(GL_TEXTURE0);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    glGenTextures(1, &textureId);
    glBindTexture(GL_TEXTURE_2D, textureId);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED,
                 static_cast<int>(w), static_cast<int>(h), 0, GL_RED, GL_UNSIGNED_BYTE, 0);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // Write glyphs into the texture
    int x = 0;
    for (int i = 32; i < 128; i++) {
        if (FT_Load_Char(face, i, FT_LOAD_RENDER))
            continue;

        glTexSubImage2D(GL_TEXTURE_2D, 0, x, 0, g->bitmap.width, g->bitmap.rows,
                        GL_RED, GL_UNSIGNED_BYTE, g->bitmap.buffer);

        float offset = static_cast<float>(x) / static_cast<float>(w);
        Character character = {
                glm::vec2(g->bitmap.width, g->bitmap.rows),
                glm::vec2(g->bitmap_left, g->bitmap_top),
                g->advance.x >> 6, // bitshift by 6 to get value in pixels (2^6 = 64)
                g->advance.y >> 6,
                offset
        };
        characters.insert(std::pair<char, Character>(i, character));

        x += g->bitmap.width;
    }

    FT_Done_Face(face);
    FT_Done_FreeType(library);
    return true;
}

bool CityNamesRenderer::prepareBuffers() {
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);

    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 12 * 6 * 4, nullptr, GL_DYNAMIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);

    // Unbind
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    return true;
}

bool CityNamesRenderer::initialize() {
    return program.build() && prepareBuffers() && prepareTextureAtlas();
}

void CityNamesRenderer::destroy() {

}

void CityNamesRenderer::render(float currentTime, t_window_definition window,
                               RenderingOptions options) {
    program.use();

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glm::mat4 projection = glm::ortho(0.0f, 800.0f, 0.0f, 600.0f);
    program.setMat4("projection", projection);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textureId);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);

    renderText("Hello World!", 25.0f, 25.0f, 1.0f, 0.8f, glm::vec3(0.5, 0.8f, 0.2f));

    // unbind
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    glDisable(GL_BLEND);
}

void CityNamesRenderer::renderText(const char *text, float x, float y,
                                   float sx, float sy, glm::vec3 color) {
    program.setVec3("textColor", color);

    struct point {
        GLfloat x;
        GLfloat y;
        GLfloat s;
        GLfloat t;
    } coords[6 * std::strlen(text)];

    int n = 0;

    for (const char *p = text; *p; p++) {
        float x2 = x + characters[*p].bearing[0] * sx;
        float y2 = -y - characters[*p].bearing[1] * sy;
        float w = characters[*p].size[0] * sx;
        float h = characters[*p].size[1] * sy;

        /* Advance the cursor to the start of the next character */
        x += characters[*p].advanceX * sx;
        y += characters[*p].advanceY * sy;

        /* Skip glyphs that have no pixels */
        if (!w || !h)
            continue;

        coords[n++] = (point) {x2, -y2,
                               characters[*p].textureOffsetX, 0};
        coords[n++] = (point) {x2 + w, -y2,
                               characters[*p].textureOffsetX + characters[*p].size[0] / atlasWidth, 0};
        coords[n++] = (point) {x2, -y2 - h,
                               characters[*p].textureOffsetX,
                               characters[*p].size[1] / atlasHeight};
        coords[n++] = (point) {x2 + w, -y2,
                               characters[*p].textureOffsetX + characters[*p].size[0] / atlasWidth, 0};
        coords[n++] = (point) {x2, -y2 - h,
                               characters[*p].textureOffsetX, characters[*p].size[1] / atlasHeight};
        coords[n++] = (point) {x2 + w, -y2 - h,
                               characters[*p].textureOffsetX + characters[*p].size[0] / atlasWidth,
                               characters[*p].size[1] / atlasHeight};
    }

    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(coords), coords);
    glDrawArrays(GL_TRIANGLES, 0, n);
}

CityNamesRenderer::CityNamesRenderer(Program &program) : program(program) {

}
