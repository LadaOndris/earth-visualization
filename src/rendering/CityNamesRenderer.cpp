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
    FT_Set_Pixel_Sizes(face, 0, 24);

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

    // Glyph information
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 12 * 6 * 4, nullptr, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);

    // Text 3D position
    glGenBuffers(1, &instanceVBO);
    glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(TextInstanceData) * 1, nullptr, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);
    glVertexAttribDivisor(1, 1); // Data is per instance

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


glm::mat4 CityNamesRenderer::constructPerspectiveProjectionMatrix(
        const Camera &camera, const Ellipsoid &ellipsoid, const t_window_definition &window) {
    // Near and far plane has to be determined from the distance to Earth
    auto closestPointOnSurface = ellipsoid.projectGeocentricPointOntoSurface(camera.getPosition());
    auto distanceToSurface = glm::length(camera.getPosition() - closestPointOnSurface);
    auto distanceToEllipsoidsCenter = glm::length(camera.getPosition() - ellipsoid.getGeocentricPosition());

    // The near plane is set in the middle of the camera position and the surface
    auto nearPlane = static_cast<float>(distanceToSurface * 0.5);
    auto farPlane = distanceToEllipsoidsCenter;

    glm::mat4 projectionMatrix;
    projectionMatrix = glm::perspective(glm::radians(camera.getFov()),
                                        (float) window.width / (float) window.height,
                                        nearPlane, farPlane);
    return projectionMatrix;
}

void CityNamesRenderer::render(float currentTime, t_window_definition window,
                               RenderingOptions options) {
    program.use();

    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glm::mat4 projectionMatrix = constructPerspectiveProjectionMatrix(camera, ellipsoid, window);
    program.setMat4("projection", projectionMatrix);
    glm::mat4 viewMatrix = camera.getViewMatrix();
    program.setMat4("view", viewMatrix);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textureId);

    glBindVertexArray(VAO);

    renderText("Hello World!", 25.0f, 25.0f, 1.0f, 1.0f, glm::vec3(0.78f, 1.0f, 1.0f));

    // unbind
    glBindVertexArray(0);

    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
}

void CityNamesRenderer::renderText(const char *text, float x, float y,
                                   float sx, float sy, glm::vec3 color) {
    /**
     * This function is based on: https://en.wikibooks.org/wiki/OpenGL_Programming/Modern_OpenGL_Tutorial_Text_Rendering_02
     *
     */
    program.setVec3("textColor", color);

    struct point {
        GLfloat x;
        GLfloat y;
        GLfloat s;
        GLfloat t;
    } coords[6 * std::strlen(text)];

    TextInstanceData positions[1];

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
    auto pointOnSurface = ellipsoid.projectGeocentricPointOntoSurface(
            glm::vec3(0.0, 0.0, 0.1));
    positions[0] = {pointOnSurface[0], pointOnSurface[1], pointOnSurface[2]};

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(coords), coords, GL_DYNAMIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(positions), positions, GL_DYNAMIC_DRAW);

    glDrawArrays(GL_TRIANGLES, 0, n);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

CityNamesRenderer::CityNamesRenderer(Program &program, Camera &camera, Ellipsoid &ellipsoid)
        : program(program), camera(camera), ellipsoid(ellipsoid) {

}
