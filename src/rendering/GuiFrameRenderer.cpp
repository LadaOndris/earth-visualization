
#include "GuiFrameRenderer.h"

#include <imgui.h>
#include <imgui_stdlib.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <iostream>

GuiFrameRenderer::GuiFrameRenderer(RenderingOptions options) : renderingOptions(options) {
}


void GuiFrameRenderer::render(float currentTime, t_window_definition window, RenderingOptions options) {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();

    ImGui::NewFrame();

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
    int width = 200;
    int height = FIT_TO_CONTENT;
    int paddingTop = 210 + 10;
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
    ImGui::Checkbox("Terrain", &renderingOptions.isTerrainEnabled);
    ImGui::Spacing();

    ImGui::End();
}

void GuiFrameRenderer::createSimulationWindow(t_window_definition window) {
    int width = 200;
    int height = FIT_TO_CONTENT;
    int paddingTop = 10;
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

    float value = 1.f;
    auto sliderFlags = ImGuiSliderFlags_None;
    if (ImGui::SliderFloat("Speed", &value, 1.f, 100.f, "%.1f", sliderFlags)) {
        std::cout << "Speed changed: " << value << std::endl;
    }

    ImGui::End();
}

void GuiFrameRenderer::createStatisticsWindow(t_window_definition window) {
    int width = 200;
    int height = FIT_TO_CONTENT;
    int paddingTop = 210 + 210 + 10;
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
    ImGui::Begin("Rendering Statistics", nullptr, windowFlags);

    ImGui::Spacing();
    ImGui::Text("Tiles: %d", renderingStatistics.numTiles);
    ImGui::Spacing();
    ImGui::Text("Frustum-culled tiles: %d", renderingStatistics.frustumCulledTiles);
    ImGui::Spacing();
    ImGui::Text("Back-faced-culled tiles: %d", renderingStatistics.backfacedCulledTiles);
    ImGui::Spacing();

    ImGui::End();
}

void GuiFrameRenderer::createCameraWindow(t_window_definition window) {

}


void GuiFrameRenderer::startOrStopSimulation() {
    renderingOptions.isSimulationRunning = ~renderingOptions.isSimulationRunning;
}

std::string GuiFrameRenderer::getCurrentSimulationTime() const {
    return "10 June, 12:45:10";
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
