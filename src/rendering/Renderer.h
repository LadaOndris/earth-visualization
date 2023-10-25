//
// Created by lada on 9/26/23.
//

#ifndef EARTH_VISUALIZATION_RENDERER_H
#define EARTH_VISUALIZATION_RENDERER_H

#include "../window_definition.h"
#include "RenderingOptions.h"

class Renderer {
public:
    virtual bool initialize() = 0;

    virtual void render(float currentTime, t_window_definition window, RenderingOptions options) = 0;

    virtual void destroy() = 0;
};

#endif //EARTH_VISUALIZATION_RENDERER_H
