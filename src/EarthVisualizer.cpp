#include "EarthVisualizer.h"

#include "rendering/CityNamesRenderer.h"
#include "rendering/SunRenderer.h"
#include "rendering/TileEarthRenderer.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <stdexcept>

EarthVisualizer::EarthVisualizer(GLFWwindow* window, t_window_definition& windowDefinition)
    : _ellipsoid(Ellipsoid::unitSphereWithCorrectRatio())
    , _camera(_ellipsoid,
              glm::vec3(-_ellipsoid.getRadii().x * 5.0f, 0.0f, 0.0f),
              glm::vec3(0.0f, 0.0f, 0.0f),
              glm::vec3(0.0f, -1.0f, 0.0f))
    , _solarSimulator(149597870700.f / 6378000.f * _ellipsoid.getRadii().x)
    , _tileContainer(_tileMeshTesselator, _dayMapAtlas, _nightMapAtlas, _heightMapAtlas, _ellipsoid)
    , _resourceManager(1000)
    , _window(window)
    , _windowDefinition(windowDefinition)
{
    float sunVsEarthRadiusFactor = 109.168105f;
    float sunRadius = sunVsEarthRadiusFactor * _ellipsoid.getRadii().x;

    _dayMapAtlas.registerAvailableTextures("textures/generated/daymaps");
    _nightMapAtlas.registerAvailableTextures("textures/generated/nightmaps");
    _heightMapAtlas.registerAvailableTextures("textures/generated/heightmaps");
    _tileContainer.setupTiles();

    RenderingOptions options = {
        .isSimulationRunning = false
    };
    _guiRenderer = std::make_shared<GuiFrameRenderer>(options, _solarSimulator);

    _tileEarthProgram.addShader(
        std::make_unique<Shader>("shaders/tiling/shader.vert", ShaderType::Vertex)
    );
    _tileEarthProgram.addShader(
        std::make_unique<Shader>("shaders/tiling/shader.tesc", ShaderType::TesselationControl)
    );
    _tileEarthProgram.addShader(
        std::make_unique<Shader>("shaders/tiling/shader.tese", ShaderType::TessellationEvaluation)
    );
    _tileEarthProgram.addShader(
        std::make_unique<Shader>("shaders/tiling/shader.frag", ShaderType::Fragment)
    );
    auto tileEarthRenderer = std::make_shared<TileEarthRenderer>(
        _tileContainer, _ellipsoid, _camera, _solarSimulator,
        _asyncTextureLoader, _resourceManager, _tileEarthProgram
    );
    tileEarthRenderer->addSubscriber(_guiRenderer);
    _renderers.push_back(tileEarthRenderer);

    _cityNamesProgram.addShader(
        std::make_unique<Shader>("shaders/text/shader.vert", ShaderType::Vertex)
    );
    _cityNamesProgram.addShader(
        std::make_unique<Shader>("shaders/text/shader.frag", ShaderType::Fragment)
    );
    auto cityNamesRenderer = std::make_shared<CityNamesRenderer>(
        _cityNamesProgram, _camera, _ellipsoid
    );
    tileEarthRenderer->addSubscriber(cityNamesRenderer);
    _renderers.push_back(cityNamesRenderer);

    _sunProgram.addShader(
        std::make_unique<Shader>("shaders/sun/shader.vert", ShaderType::Vertex)
    );
    _sunProgram.addShader(
        std::make_unique<Shader>("shaders/sun/shader.frag", ShaderType::Fragment)
    );
    auto sunRenderer = std::make_shared<SunRenderer>(
        _camera, _solarSimulator, sunRadius, _sunProgram
    );
    _renderers.push_back(sunRenderer);
    _renderers.push_back(_guiRenderer);

    initializeRenderers();
}

EarthVisualizer::~EarthVisualizer() {
    for (const auto& renderer : _renderers)
        renderer->destroy();
}

EarthCenteredCamera& EarthVisualizer::getCamera() {
    return _camera;
}

void EarthVisualizer::initializeRenderers() {
    for (const auto& renderer : _renderers) {
        if (!renderer->initialize())
            throw std::runtime_error("Failed to initialize a renderer");
    }
}

void EarthVisualizer::run() {
    glViewport(0, 0, _windowDefinition.width, _windowDefinition.height);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_MULTISAMPLE);

    float lastFrameTime = static_cast<float>(glfwGetTime());
    bool simulationRunningLastFrame = false;
    _solarSimulator.updateSunPosition(
        0, static_cast<float>(_guiRenderer->getRenderingOptions().simulationSpeed)
    );

    while (!glfwWindowShouldClose(_window)) {
        RenderingOptions options = _guiRenderer->getRenderingOptions();
        auto currentFrameTime = static_cast<float>(glfwGetTime());

        if (options.isSimulationRunning) {
            if (simulationRunningLastFrame) {
                float additionalFrameTime = currentFrameTime - lastFrameTime;
                _solarSimulator.updateSunPosition(
                    additionalFrameTime, static_cast<float>(options.simulationSpeed)
                );
            } else {
                simulationRunningLastFrame = true;
            }
            lastFrameTime = currentFrameTime;
        } else {
            simulationRunningLastFrame = false;
        }

        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        for (const auto& renderer : _renderers)
            renderer->render(currentFrameTime, _windowDefinition, options);

        glfwSwapBuffers(_window);
        glfwPollEvents();
    }
}
