
#include "GuiFrameRenderer.h"

#include <imgui.h>
#include <imgui_stdlib.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <iostream>
#include <iomanip>

GuiFrameRenderer::GuiFrameRenderer(RenderingOptions options, const SolarSimulator &simulator)
        : renderingOptions(options), simulator(simulator) {
}


void GuiFrameRenderer::render(float currentTime, t_window_definition window, RenderingOptions options) {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();

    ImGui::NewFrame();

    paddingTop = paddingBetweenWindows;
    createSimulationWindow(window);
    createFeaturesWindow(window);
    createStatisticsWindow(window);
    createCameraWindow(window);

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

/**
 * Creates a window that enables turning on and off features.
 */
void GuiFrameRenderer::createFeaturesWindow(t_window_definition window) {
    int width = windowWidth;
    int height = FIT_TO_CONTENT;
    int paddingRight = 10;

    // Window position
    auto xPos = static_cast<float>(window.width - width - paddingRight);
    auto yPos = static_cast<float>(paddingTop);
    auto windowPosition = ImVec2(xPos, yPos);
    ImGui::SetNextWindowPos(windowPosition, ImGuiCond_Always);
    // Window size
    ImGui::SetNextWindowSize(
            ImVec2(static_cast<float>(width), static_cast<float>(height)),
            ImGuiCond_Always
    );
    // Set window opacity
    ImGui::SetNextWindowBgAlpha(0.7f);

    auto windowFlags = ImGuiWindowFlags_NoResize |
                       ImGuiWindowFlags_NoMove |
                       ImGuiWindowFlags_NoScrollbar |
                       ImGuiWindowFlags_NoCollapse;
    ImGui::Begin("Turn on/off features", nullptr, windowFlags);

    ImGui::Spacing();
    ImGui::Checkbox("Wireframe", &renderingOptions.isWireframeEnabled);
    ImGui::Spacing();
    ImGui::Checkbox("Color texture", &renderingOptions.isTextureEnabled);
    ImGui::Spacing();
    ImGui::Checkbox("Night", &renderingOptions.isNightEnabled);
    ImGui::Spacing();
    ImGui::Checkbox("Terrain", &renderingOptions.isTerrainEnabled);
    ImGui::Spacing();
    ImGui::Checkbox("Grid", &renderingOptions.isGridEnabled);
    ImGui::Spacing();

    ImGui::End();
    float windowHeight = 180;
    updateTopPadding(windowHeight);
}

void GuiFrameRenderer::createSimulationWindow(t_window_definition window) {
    int width = windowWidth;
    int height = FIT_TO_CONTENT;
    int paddingRight = 10;

    // Window position
    auto xPos = static_cast<float>(window.width - width - paddingRight);
    auto yPos = static_cast<float>(paddingTop);
    auto windowPosition = ImVec2(xPos, yPos);
    ImGui::SetNextWindowPos(windowPosition, ImGuiCond_Always);
    // Window size
    ImGui::SetNextWindowSize(
            ImVec2(static_cast<float>(width), static_cast<float>(height)),
            ImGuiCond_Always
    );
    // Set window opacity
    ImGui::SetNextWindowBgAlpha(0.7f);

    auto windowFlags = ImGuiWindowFlags_NoResize |
                       ImGuiWindowFlags_NoMove |
                       ImGuiWindowFlags_NoScrollbar |
                       ImGuiWindowFlags_NoCollapse;
    ImGui::Begin("Simulation", nullptr, windowFlags);

    ImGui::Spacing();
    ImGui::Text("%s", getCurrentSimulationTime().c_str());
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    auto simulationButtonText = renderingOptions.isSimulationRunning ? "Stop" : "Start";
    if (ImGui::Button(simulationButtonText)) {
        startOrStopSimulation();
    }

    auto sliderFlags = ImGuiSliderFlags_None;
    ImGui::SliderInt("Speed", &renderingOptions.simulationSpeed, 1, 86400, "%d", sliderFlags);

    ImGui::End();
    float windowHeight = 150;
    updateTopPadding(windowHeight);
}

void GuiFrameRenderer::createStatisticsWindow(t_window_definition window) {
    int width = windowWidth;
    int height = FIT_TO_CONTENT;
    int paddingRight = 10;

    // Window position
    auto xPos = static_cast<float>(window.width - width - paddingRight);
    auto yPos = static_cast<float>(paddingTop);
    auto windowPosition = ImVec2(xPos, yPos);
    ImGui::SetNextWindowPos(windowPosition, ImGuiCond_Always);
    // Window size
    ImGui::SetNextWindowSize(
            ImVec2(static_cast<float>(width), static_cast<float>(height)),
            ImGuiCond_Always
    );
    // Set window opacity
    ImGui::SetNextWindowBgAlpha(0.7f);

    auto windowFlags = ImGuiWindowFlags_NoResize |
                       ImGuiWindowFlags_NoMove |
                       ImGuiWindowFlags_NoScrollbar;
    ImGui::Begin("Rendering Statistics", nullptr, windowFlags);

    ImGui::Spacing();
    ImGui::Text("\tTiles");
    ImGui::Spacing();
    ImGui::Text("Tiles: %d", renderingStatistics.numTiles);
    ImGui::Spacing();
    ImGui::Text("Frustum-culled tiles: %d", renderingStatistics.frustumCulledTiles);
    ImGui::Spacing();
    ImGui::Text("Back-faced-culled tiles: %d", renderingStatistics.backfacedCulledTiles);
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();
    ImGui::Text("\tTextures");
    ImGui::Spacing();
    ImGui::Text("Loaded textures: %d", renderingStatistics.loadedTextures);
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();
    ImGui::Text("\tCamera");
    ImGui::Spacing();
    ImGui::Text("Longitude: %.3f°", renderingStatistics.cameraPosition[0] * TO_DEGS_COEFF);
    ImGui::Spacing();
    ImGui::Text("Latitude: %.3f°", renderingStatistics.cameraPosition[1] * TO_DEGS_COEFF);
    ImGui::Spacing();
    ImGui::Text("Altitude: %.2f km", renderingStatistics.cameraPosition[2] / 1000);
    ImGui::Spacing();

    ImGui::End();
    float windowHeight = 100;
    updateTopPadding(windowHeight);
}

void GuiFrameRenderer::updateTopPadding(float yPosWindow) {
    // Calculate the height based on content
    float windowHeight = yPosWindow;//ImGui::GetCursorPosY() - yPosWindow;
    // Update paddingTop for the next window
    paddingTop += windowHeight + paddingBetweenWindows;
}

void GuiFrameRenderer::createCameraWindow(t_window_definition window) {

}


void GuiFrameRenderer::startOrStopSimulation() {
    renderingOptions.isSimulationRunning = !renderingOptions.isSimulationRunning;
}

std::string GuiFrameRenderer::getCurrentSimulationTime() const {
    std::tm datetime = simulator.getCurrentSimulationTime();

    // Conver to the following format: "10 June, 12:45:10"
    std::stringstream ss;
    ss << std::put_time(&datetime, "%d %B, %H:%M:%S");
    return ss.str();
}

RenderingOptions GuiFrameRenderer::getRenderingOptions() const {
    return renderingOptions;
}

bool GuiFrameRenderer::initialize() {
    return true;
}

void GuiFrameRenderer::destroy() {

}

void GuiFrameRenderer::notify(RenderingStatistics statistics) {
    renderingStatistics = statistics;
}
