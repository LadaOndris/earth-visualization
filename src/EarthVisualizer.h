#pragma once

struct GLFWwindow;

#include "cameras/EarthCenteredCamera.h"
#include "ellipsoid.h"
#include "program.h"
#include "rendering/GuiFrameRenderer.h"
#include "rendering/Renderer.h"
#include "resources/AsyncTextureLoader.h"
#include "resources/ResourceManager.h"
#include "simulation/SolarSimulator.h"
#include "tesselation/TileMeshTesselator.h"
#include "textures/TextureAtlas.h"
#include "tiling/TileContainer.h"
#include "window_definition.h"

#include <memory>
#include <vector>

class EarthVisualizer {
public:
    // Throws std::runtime_error if any renderer fails to initialize.
    EarthVisualizer(GLFWwindow* window, t_window_definition& windowDefinition);
    ~EarthVisualizer();

    EarthVisualizer(const EarthVisualizer&) = delete;
    EarthVisualizer& operator=(const EarthVisualizer&) = delete;

    void run();
    [[nodiscard]] EarthCenteredCamera& getCamera();

private:
    void initializeRenderers();

    // Initialization order is critical: members are constructed in declaration order.
    Ellipsoid _ellipsoid;
    EarthCenteredCamera _camera;     // holds ref to _ellipsoid
    SolarSimulator _solarSimulator;  // needs ellipsoid radii for sun distance

    TileMeshTesselator _tileMeshTesselator;
    TextureAtlas _dayMapAtlas;
    TextureAtlas _nightMapAtlas;
    TextureAtlas _heightMapAtlas;
    TileContainer _tileContainer;    // holds refs to _tileMeshTesselator, atlases, _ellipsoid

    AsyncTextureLoader _asyncTextureLoader;
    ResourceManager _resourceManager;

    Program _tileEarthProgram;
    Program _cityNamesProgram;
    Program _sunProgram;

    std::shared_ptr<GuiFrameRenderer> _guiRenderer;
    std::vector<std::shared_ptr<Renderer>> _renderers;

    GLFWwindow* _window;
    t_window_definition& _windowDefinition;
};
