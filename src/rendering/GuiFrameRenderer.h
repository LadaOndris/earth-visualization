//
// Created by lada on 9/28/23.
//

#ifndef EARTH_VISUALIZATION_GUIFRAMERENDERER_H
#define EARTH_VISUALIZATION_GUIFRAMERENDERER_H


#include <string>
#include "Renderer.h"
#include "RenderingOptions.h"

class GuiFrameRenderer : public Renderer {
private:
    int FIT_TO_CONTENT = 0;
    RenderingOptions renderingOptions;

    void createSimulationWindow(t_window_definition window);

    void createFeaturesWindow(t_window_definition window);

    void createCameraWindow(t_window_definition window);

    void startOrStopSimulation();

    std::string getCurrentSimulationTime() const;


public:
    explicit GuiFrameRenderer(RenderingOptions options);

    bool initialize() override;

    void destroy() override;

    void render(float currentTime, t_window_definition window, RenderingOptions options) override;

    RenderingOptions getRenderingOptions() const;
};


#endif //EARTH_VISUALIZATION_GUIFRAMERENDERER_H
