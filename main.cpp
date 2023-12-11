#include <iostream>
#include <cmath>
#include <utility>
#include <memory>

#include "include/glad/glad.h"
#include <GLFW/glfw3.h>

#include "include/program.h"
#include "src/cameras/FreeCamera.h"
#include "src/ellipsoid.h"
#include "src/tesselation/SubdivisionSphereTesselator.h"
#include "src/vertex.h"
#include "src/window_definition.h"
#include "src/rendering/SunRenderer.h"
#include "src/rendering/GuiFrameRenderer.h"
#include "src/cameras/EarthCenteredCamera.h"
#include "src/tiling/TileContainer.h"
#include "src/rendering/TileEarthRenderer.h"
#include "src/simulation/SolarSimulator.h"
#include "src/rendering/CityNamesRenderer.h"

#include <glm/vec3.hpp> // glm::vec3
#include <glm/vec4.hpp> // glm::vec4
#include <glm/mat4x4.hpp> // glm::mat4
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <imgui.h>
#include <imgui_stdlib.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <chrono>
#include <array>
#include <algorithm>
#include <future>

t_window_definition windowDefinition;
float lastX = 400, lastY = 300;
bool firstMouseMove = true;
bool lbutton_down = false;
GLFWwindow *window = nullptr;


Ellipsoid ellipsoid = Ellipsoid::unitSphereWithCorrectRatio();
auto radii = ellipsoid.getRadii();
// From the side of the Earth
//Camera camera(5.0f, glm::vec3(-radii.x * 5, 0, 0),
//              0, 55);
// From above the Earth
//Camera camera(5.0f, glm::vec3(0, radii.y * 5, 0), -90.f);

EarthCenteredCamera camera(ellipsoid,
                           glm::vec3(-radii.x * 5, 0, 0),
                           glm::vec3(0, 0, 0),
                           glm::vec3(0, -1, 0));

void error_callback(int error, const char *description) {
    fprintf(stderr, "Error: %s\n", description);
}

void framebuffer_size_callback(GLFWwindow *window, int width, int height) {
    std::cout << "Window resized to " + std::to_string(width) + "/" + std::to_string(height) << std::endl;
    glViewport(0, 0, width, height);
    windowDefinition.width = width;
    windowDefinition.height = height;
}

static void mouse_button_callback(GLFWwindow *window, int button, int action, int mods) {
    auto &io = ImGui::GetIO();
    if (io.WantCaptureMouse) {
        return;
    }

    if (button == GLFW_MOUSE_BUTTON_LEFT) {
        if (GLFW_PRESS == action)
            lbutton_down = true;
        else if (GLFW_RELEASE == action)
            lbutton_down = false;
    }

    if (lbutton_down) {

    }
}

void cursor_pos_callback(GLFWwindow *window, double xpos, double ypos) {
    auto &io = ImGui::GetIO();
    if (io.WantCaptureMouse) {
        return;
    }

    if (firstMouseMove) // initially set to true
    {
        lastX = xpos;
        lastY = ypos;
        firstMouseMove = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // reversed since y-coordinates range from bottom to top
    lastX = xpos;
    lastY = ypos;

    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
        camera.onMouseDrag(xoffset, yoffset);
    }

    camera.onMouseMove(xoffset, yoffset);
}

void scroll_callback(GLFWwindow *window, double xoffset, double yoffset) {
    auto &io = ImGui::GetIO();
    if (io.WantCaptureMouse) {
        return;
    }

    camera.onMouseScroll(xoffset, yoffset);
}

template<typename T, std::size_t N>
bool contains(const std::array<T, N> &arr, const T &value) {
    return std::find(std::begin(arr), std::end(arr), value) != std::end(arr);
}

static void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods) {
    if (action == GLFW_PRESS && key == GLFW_KEY_ESCAPE) {
        glfwSetWindowShouldClose(window, true);
    }

    std::array<int, 2> zoomInKeys{GLFW_KEY_KP_ADD, GLFW_KEY_UP};
    std::array<int, 2> zoomOutKeys = {GLFW_KEY_KP_SUBTRACT, GLFW_KEY_DOWN};
    int rotateLeftKey = GLFW_KEY_A;
    int rotateRightKey = GLFW_KEY_D;
    int rotateUpKey = GLFW_KEY_W;
    int rotateDownKey = GLFW_KEY_S;

    int zoomSpped = 1;
    int rotateSpeed = 20;

    if (action == GLFW_PRESS || action == GLFW_REPEAT) {
        if (contains(zoomInKeys, key)) {
            camera.onMouseScroll(0, zoomSpped);
        }
        if (contains(zoomOutKeys, key)) {
            camera.onMouseScroll(0, -zoomSpped);
        }
        if (key == rotateLeftKey) {
            camera.onMouseDrag(rotateSpeed, 0);
        }
        if (key == rotateRightKey) {
            camera.onMouseDrag(-rotateSpeed, 0);
        }
        if (key == rotateUpKey) {
            camera.onMouseDrag(0, -rotateSpeed);
        }
        if (key == rotateDownKey) {
            camera.onMouseDrag(0, rotateSpeed);
        }
    }
}

/**
 * Creates the window, callbacks, etc.
 */
bool initializeGlfw() {
    glfwSetErrorCallback(error_callback);

    if (!glfwInit()) {
        std::cerr << "[ERROR] Couldn't initialize GLFW" << std::endl;
        return false;
    } else {
        std::cout << "[INFO] GLFW initialized" << std::endl;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    // Multi-sampling
    glfwWindowHint(GLFW_SAMPLES, 4);

    window = glfwCreateWindow(windowDefinition.width, windowDefinition.height, "Earth Viewer", NULL, NULL);
    if (!window) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, cursor_pos_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetKeyCallback(window, key_callback);
    return true;
}

bool initializeGlad() {
    glfwMakeContextCurrent(window);
    if (!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress)) {
        std::cout << "[ERROR] Failed to initialize GLAD" << std::endl;
        return false;
    } else {
        std::cout << "[INFO] GLAD initialized" << std::endl;
    }
    return true;
}

bool initializeImgui() {
    std::string fontName = "JetBrainsMono-ExtraLight.ttf";
    float highDPIscaleFactor = 1.0;

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    ImGuiIO &io = ImGui::GetIO();
    (void) io;

//    io.Fonts->AddFontFromFileTTF(
//            fontName.c_str(),
//            24.0f * highDPIscaleFactor,
//            NULL,
//            NULL
//    );
    // setImGuiStyle(highDPIscaleFactor);

    if (!ImGui_ImplGlfw_InitForOpenGL(window, true)) {
        std::cout << "[ERROR] Failed to initialize ImGui (ImGui_ImplGlfw_InitForOpenGL)" << std::endl;
        return false;
    }
    if (!ImGui_ImplOpenGL3_Init()) {
        std::cout << "[ERROR] Failed to initialize ImGui (ImGui_ImplOpenGL3_Init)" << std::endl;
        return false;
    }

    std::cout << "[INFO] IMGUI initialized" << std::endl;
    return true;
}

std::string formatTime(const std::chrono::system_clock::time_point &timePoint) {
    // Convert time point to a time_t
    std::time_t time = std::chrono::system_clock::to_time_t(timePoint);

    // Convert time_t to a struct tm
    std::tm tmStruct = *std::localtime(&time);

    // Format the struct tm into a string
    char buffer[80];
    std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", &tmStruct);

    return buffer;
}

bool initializeRenderers(std::vector<std::shared_ptr<Renderer>> renderers) {
    for (const auto &renderer: renderers) {
        bool initializationResult = renderer->initialize();
        if (!initializationResult) {
            return false;
        }
    }
    return true;
}

void startRendering(const std::vector<std::shared_ptr<Renderer>> &renderers,
                    const std::shared_ptr<GuiFrameRenderer> &guiRenderer,
                    SolarSimulator &solarSimulator) {
    glViewport(0, 0, windowDefinition.width, windowDefinition.height);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_MULTISAMPLE);

    float lastFrameTime = static_cast<float>(glfwGetTime());
    bool simulationRunningLastFrame = false;
    // Initialize the Sun position
    solarSimulator.updateSunPosition(0, static_cast<float>(guiRenderer->getRenderingOptions().simulationSpeed));

    while (!glfwWindowShouldClose(window)) {
        RenderingOptions options = guiRenderer->getRenderingOptions();
        auto currentFrameTime = static_cast<float>(glfwGetTime());

        if (options.isSimulationRunning) {
            if (simulationRunningLastFrame) {
                float additionalFrameTime = currentFrameTime - lastFrameTime;
                solarSimulator.updateSunPosition(additionalFrameTime, static_cast<float>(options.simulationSpeed));
            } else {
                simulationRunningLastFrame = true;
            }

            lastFrameTime = currentFrameTime;
        } else {
            simulationRunningLastFrame = false;
        }

        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        for (const auto &renderer: renderers) {
            renderer->render(currentFrameTime, windowDefinition, options);
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }
}

void cleanup(const std::vector<std::shared_ptr<Renderer>> &renderers) {
    for (const auto &renderer: renderers) {
        renderer->destroy();
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();
}

void resourceLoaderThreadStart() {
    ResourceLoader loader;
    loader.start();
}


/**
 * From:
 * https://github.com/yuzu-emu/yuzu/blob/875568bb3e34725578f7fa3661c8bad89f23a173/src/video_core/renderer_opengl/renderer_opengl.cpp#L82
 */
const char *getSource(GLenum source) {
    switch (source) {
        case GL_DEBUG_SOURCE_API:
            return "API";
        case GL_DEBUG_SOURCE_WINDOW_SYSTEM:
            return "WINDOW_SYSTEM";
        case GL_DEBUG_SOURCE_SHADER_COMPILER:
            return "SHADER_COMPILER";
        case GL_DEBUG_SOURCE_THIRD_PARTY:
            return "THIRD_PARTY";
        case GL_DEBUG_SOURCE_APPLICATION:
            return "APPLICATION";
        case GL_DEBUG_SOURCE_OTHER:
            return "OTHER";
        default:
            return "Unknown source";
    }
}

/**
 * From:
 * https://github.com/yuzu-emu/yuzu/blob/875568bb3e34725578f7fa3661c8bad89f23a173/src/video_core/renderer_opengl/renderer_opengl.cpp#L102
 */
const char *getType(GLenum type) {
    switch (type) {
        case GL_DEBUG_TYPE_ERROR:
            return "ERROR";
        case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
            return "DEPRECATED_BEHAVIOR";
        case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
            return "UNDEFINED_BEHAVIOR";
        case GL_DEBUG_TYPE_PORTABILITY:
            return "PORTABILITY";
        case GL_DEBUG_TYPE_PERFORMANCE:
            return "PERFORMANCE";
        case GL_DEBUG_TYPE_OTHER:
            return "OTHER";
        case GL_DEBUG_TYPE_MARKER:
            return "MARKER";
        default:
            return "Unknown type";
    }
}

/**
 * From:
 * https://github.com/yuzu-emu/yuzu/blob/875568bb3e34725578f7fa3661c8bad89f23a173/src/video_core/renderer_opengl/renderer_opengl.cpp#L102
 */
void APIENTRY processErrorMessageCallback(
        GLenum source,
        GLenum type,
        GLuint id,
        GLenum severity,
        GLsizei length,
        const GLchar *message,
        const void *userParam
) {
    const char format[] = "%s %s %u: %s";
    const char *const str_source = getSource(source);
    const char *const str_type = getType(type);

    fprintf(stderr, format, str_source, str_type, id, message);
}


void mainAppThread(std::promise<int> &&returnCodePromise) {
    if (!initializeGlfw() || !initializeGlad() || !initializeImgui()) {
        returnCodePromise.set_value(EXIT_FAILURE);
        return;
    }
    glDebugMessageCallback(processErrorMessageCallback, nullptr);

    auto sunVsEarthRadiusFactor = 109.168105; // Sun_radius / Earth_radius
    auto sunRadius = sunVsEarthRadiusFactor * radii.x;
    auto sunDistanceMeters = 149597870700.f;
    auto earthRadiusMeters = 6378000.f;
    auto sunDistance = sunDistanceMeters / earthRadiusMeters * radii.x;
    //auto lightPosition = glm::vec3(0.0f, -sunDistance, sunDistance);
    SolarSimulator solarSimulator(sunDistance);

    std::vector<std::shared_ptr<Renderer>> renderers;

    SubdivisionSphereTesselator subdivisionSurfaces;

    TileMeshTesselator tileMeshTesselator;
    TextureAtlas dayMapAtlas;
    TextureAtlas nightMapAtlas;
    TextureAtlas heightMapAtlas;
    TileContainer tileContainer(tileMeshTesselator, dayMapAtlas,
                                nightMapAtlas, heightMapAtlas, ellipsoid);

    ResourceFetcher resourceFetcher;
    ResourceManager resourceManager(1000);

    dayMapAtlas.registerAvailableTextures("textures/daymaps");
    nightMapAtlas.registerAvailableTextures("textures/nightmaps");
    heightMapAtlas.registerAvailableTextures("textures/heightmaps");
    tileContainer.setupTiles();

    RenderingOptions options = {
            .isSimulationRunning = false
    };
    auto guiRenderer =
            std::make_shared<GuiFrameRenderer>(options, solarSimulator);

    Program tileEarthRendererProgram;
    tileEarthRendererProgram.addShader(
            std::make_unique<Shader>("shaders/tiling/shader.vert", ShaderType::Vertex)
    );
    tileEarthRendererProgram.addShader(
            std::make_unique<Shader>("shaders/tiling/shader.tesc", ShaderType::TesselationControl)
    );
    tileEarthRendererProgram.addShader(
            std::make_unique<Shader>("shaders/tiling/shader.tese", ShaderType::TessellationEvaluation)
    );
    tileEarthRendererProgram.addShader(
            std::make_unique<Shader>("shaders/tiling/shader.frag", ShaderType::Fragment)
    );
    auto tileEarthRenderer =
            std::make_shared<TileEarthRenderer>(
                    tileContainer, ellipsoid, camera, solarSimulator,
                    resourceFetcher, resourceManager, tileEarthRendererProgram
            );
    tileEarthRenderer->addSubscriber(guiRenderer);
    renderers.push_back(tileEarthRenderer);

    Program cityNamesRendererProgram;
    cityNamesRendererProgram.addShader(
            std::make_unique<Shader>("shaders/text/shader.vert", ShaderType::Vertex)
    );
    cityNamesRendererProgram.addShader(
            std::make_unique<Shader>("shaders/text/shader.frag", ShaderType::Fragment)
    );
    auto cityNamesRenderer =
            std::make_shared<CityNamesRenderer>(cityNamesRendererProgram, camera, ellipsoid);
    renderers.push_back(cityNamesRenderer);


    Program sunRendererProgram;
    sunRendererProgram.addShader(
            std::make_unique<Shader>("shaders/sun/shader.vert", ShaderType::Vertex)
    );
    sunRendererProgram.addShader(
            std::make_unique<Shader>("shaders/sun/shader.frag", ShaderType::Fragment)
    );
    auto sunRenderer =
            std::make_shared<SunRenderer>(camera, solarSimulator, sunRadius, sunRendererProgram);

    renderers.push_back(sunRenderer);
    renderers.push_back(guiRenderer);

    bool result = initializeRenderers(renderers);
    if (!result) {
        cleanup(renderers);
        returnCodePromise.set_value(EXIT_FAILURE);
        return;
    }

    startRendering(renderers, guiRenderer, solarSimulator);
    cleanup(renderers);

    returnCodePromise.set_value(EXIT_SUCCESS);
}

int main() {
    std::cout << "Starting the application..." << std::endl;
    std::cout << "Starting application thread: " << std::this_thread::get_id() << std::endl;

    std::thread loaderThread(resourceLoaderThreadStart);

    std::promise<int> p;
    auto futureReturnCode = p.get_future();
    std::thread mainThread(mainAppThread, std::move(p));

    mainThread.join();
    int returnCode = futureReturnCode.get();

    // Stop loader thread
    stopThread = true;
    cv.notify_all();
    loaderThread.join();

    return returnCode;
}


