
#pragma once

#include "Renderer.h"
#include "RenderingOptions.h"
#include "RendererSubscriber.h"
#include "simulation/SolarSimulator.h"

#include <string>

class GuiFrameRenderer : public Renderer, public RendererSubscriber {
private:
    int windowWidth = 230;
    float paddingBetweenWindows = 10;
    float paddingTop = paddingBetweenWindows;
    int FIT_TO_CONTENT = 0;
    RenderingOptions _renderingOptions;
    RenderingStatistics renderingStatistics;
    float TO_DEGS_COEFF = 180.0f / 3.14159265f;
    const SolarSimulator &_simulator;

    void createSimulationWindow(t_window_definition window);

    void createFeaturesWindow(t_window_definition window);

    void createStatisticsWindow(t_window_definition window);

    void createCameraWindow(t_window_definition window);

    void startOrStopSimulation();

    std::string getCurrentSimulationTime() const;

    void updateTopPadding(float yPosWindow);
public:
    explicit GuiFrameRenderer(RenderingOptions options, const SolarSimulator &simulator);

    void render(float currentTime, t_window_definition window, RenderingOptions options) override;

    RenderingOptions getRenderingOptions() const;

    void notify(RenderingStatistics renderingStatistics) override;
};
