//
// Created by lada on 9/28/23.
//

#include "GuiFrameRenderer.h"

#include <imgui.h>
#include <imgui_stdlib.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <iostream>

GuiFrameRenderer::GuiFrameRenderer(bool simulationIsRunning)
        : simulationIsRunning(simulationIsRunning) {
}


void GuiFrameRenderer::render(float currentTime, t_window_definition window) {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();

    ImGui::NewFrame();

    createSimulationWindow(window);
    createCameraWindow(window);

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void GuiFrameRenderer::createSimulationWindow(t_window_definition window) {
    int width = 200;
    int height = 100;
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

    auto simulationButtonText = simulationIsRunning ? "Stop" : "Start";
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

void GuiFrameRenderer::createCameraWindow(t_window_definition window) {

}


void GuiFrameRenderer::startOrStopSimulation() {
    simulationIsRunning = ~simulationIsRunning;
}

std::string GuiFrameRenderer::getCurrentSimulationTime() {
    return "10 June, 12:45:10";
}
