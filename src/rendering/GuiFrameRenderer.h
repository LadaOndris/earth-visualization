//
// Created by lada on 9/28/23.
//

#ifndef EARTH_VISUALIZATION_GUIFRAMERENDERER_H
#define EARTH_VISUALIZATION_GUIFRAMERENDERER_H


#include <string>
#include "Renderer.h"

class GuiFrameRenderer : public Renderer {
private:
    bool simulationIsRunning;

    void createSimulationWindow(t_window_definition window);

    void createCameraWindow(t_window_definition window);

    void startOrStopSimulation();

    std::string getCurrentSimulationTime();

public:
    explicit GuiFrameRenderer(bool simulationIsRunning);

    void render(float currentTime, t_window_definition window) override;
};


#endif //EARTH_VISUALIZATION_GUIFRAMERENDERER_H
