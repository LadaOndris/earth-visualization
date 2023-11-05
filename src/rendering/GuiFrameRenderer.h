//
// Created by lada on 9/28/23.
//

#ifndef EARTH_VISUALIZATION_GUIFRAMERENDERER_H
#define EARTH_VISUALIZATION_GUIFRAMERENDERER_H


#include <string>
#include "Renderer.h"
#include "RenderingOptions.h"
#include "RendererSubscriber.h"
#include "../simulation/SolarSimulator.h"

class GuiFrameRenderer : public Renderer, public RendererSubscriber {
private:
    int windowWidth = 230;
    float paddingBetweenWindows = 10;
    float paddingTop = paddingBetweenWindows;
    int FIT_TO_CONTENT = 0;
    RenderingOptions renderingOptions;
    RenderingStatistics renderingStatistics;
    float TO_DEGS_COEFF = 180 / 3.14159265;
    const SolarSimulator &simulator;

    void createSimulationWindow(t_window_definition window);

    void createFeaturesWindow(t_window_definition window);

    void createStatisticsWindow(t_window_definition window);

    void createCameraWindow(t_window_definition window);

    void startOrStopSimulation();

    std::string getCurrentSimulationTime() const;

    void updateTopPadding(float yPosWindow);
public:
    explicit GuiFrameRenderer(RenderingOptions options, const SolarSimulator &simulator);

    bool initialize() override;

    void destroy() override;

    void render(float currentTime, t_window_definition window, RenderingOptions options) override;

    RenderingOptions getRenderingOptions() const;

    void notify(RenderingStatistics renderingStatistics) override;
};


#endif //EARTH_VISUALIZATION_GUIFRAMERENDERER_H
