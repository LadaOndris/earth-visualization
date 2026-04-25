
#pragma once

#include "window_definition.h"
#include "RenderingOptions.h"

class Renderer {
public:
    virtual ~Renderer() = default;

    virtual void render(float currentTime, t_window_definition window, RenderingOptions options) = 0;
};
