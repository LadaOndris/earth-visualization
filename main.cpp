
#include "include/glad/glad.h"
#include <GLFW/glfw3.h>

#include "include/program.h"
#include "src/ellipsoid.h"
#include "src/tesselation/SubdivisionSphereTesselator.h"
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

#include <unistd.h>
#include <thread>
#include <iostream>
#include <cmath>
#include <memory>

t_window_definition gWindowDefinition{800, 600};
float gLastX = 400, gLastY = 300;
bool gFirstMouseMove = true;
bool gLbuttonDown = false;
GLFWwindow *gWindow = nullptr;


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
    gWindowDefinition.width = width;
    gWindowDefinition.height = height;
}

static void mouse_button_callback(GLFWwindow *window, int button, int action, int mods) {
    auto &io = ImGui::GetIO();
    if (io.WantCaptureMouse) {
        return;
    }

    if (button == GLFW_MOUSE_BUTTON_LEFT) {
        if (GLFW_PRESS == action)
            gLbuttonDown = true;
        else if (GLFW_RELEASE == action)
            gLbuttonDown = false;
    }

    if (gLbuttonDown) {

    }
}

void cursor_pos_callback(GLFWwindow *window, double xpos, double ypos) {
    auto &io = ImGui::GetIO();
    if (io.WantCaptureMouse) {
        return;
    }

    if (gFirstMouseMove) // initially set to true
    {
        gLastX = xpos;
        gLastY = ypos;
        gFirstMouseMove = false;
    }

    float xoffset = xpos - gLastX;
    float yoffset = gLastY - ypos; // reversed since y-coordinates range from bottom to top
    gLastX = xpos;
    gLastY = ypos;

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

    // Check for display availability to prevent hanging
    if (!getenv("DISPLAY") && !getenv("WAYLAND_DISPLAY")) {
        std::cerr << "[ERROR] No display server detected (DISPLAY or WAYLAND_DISPLAY not set). "
                  << "This application requires a GUI environment to run." << std::endl;
        glfwTerminate();
        return false;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwWindowHint(GLFW_DOUBLEBUFFER, GLFW_TRUE);
    glfwWindowHint(GLFW_FOCUSED, GLFW_TRUE);
    glfwWindowHint(GLFW_VISIBLE, GLFW_TRUE);

    gWindow = glfwCreateWindow(gWindowDefinition.width, gWindowDefinition.height, "Earth Viewer", NULL, NULL);
    if (!gWindow) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    glfwSetFramebufferSizeCallback(gWindow, framebuffer_size_callback);
    glfwSetCursorPosCallback(gWindow, cursor_pos_callback);
    glfwSetMouseButtonCallback(gWindow, mouse_button_callback);
    glfwSetScrollCallback(gWindow, scroll_callback);
    glfwSetKeyCallback(gWindow, key_callback);
    return true;
}

bool initializeGlad() {
    if (!gWindow) {
        std::cerr << "[ERROR] window is null, cannot initialize GLAD" << std::endl;
        return false;
    }
    glfwMakeContextCurrent(gWindow);
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

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    ImGuiIO &io = ImGui::GetIO();
    (void) io;

    // float highDPIscaleFactor = 1.0;
//    io.Fonts->AddFontFromFileTTF(
//            fontName.c_str(),
//            24.0f * highDPIscaleFactor,
//            NULL,
//            NULL
//    );
    // setImGuiStyle(highDPIscaleFactor);

    if (!ImGui_ImplGlfw_InitForOpenGL(gWindow, true)) {
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
    glViewport(0, 0, gWindowDefinition.width, gWindowDefinition.height);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_MULTISAMPLE);

    float lastFrameTime = static_cast<float>(glfwGetTime());
    bool simulationRunningLastFrame = false;
    // Initialize the Sun position
    solarSimulator.updateSunPosition(0, static_cast<float>(guiRenderer->getRenderingOptions().simulationSpeed));

    while (!glfwWindowShouldClose(gWindow)) {
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
            renderer->render(currentFrameTime, gWindowDefinition, options);
        }

        glfwSwapBuffers(gWindow);
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

    glfwDestroyWindow(gWindow);
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
    if (severity == GL_DEBUG_SEVERITY_NOTIFICATION) {
        return;
    }

    const char *severityStr = "";
    switch (severity) {
        case GL_DEBUG_SEVERITY_HIGH:   severityStr = "HIGH";   break;
        case GL_DEBUG_SEVERITY_MEDIUM: severityStr = "MEDIUM"; break;
        case GL_DEBUG_SEVERITY_LOW:    severityStr = "LOW";    break;
    }

    fprintf(stderr, "[GL %s] %s %s %u: %s\n",
            severityStr, getSource(source), getType(type), id, message);
}


int mainAppThread() {
    if (!initializeGlfw() || !initializeGlad() || !initializeImgui()) {
        return EXIT_FAILURE;
    }
    if (glDebugMessageCallback) {
        glDebugMessageCallback(processErrorMessageCallback, nullptr);
        glEnable(GL_DEBUG_OUTPUT);
    }

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

    dayMapAtlas.registerAvailableTextures("textures/generated/daymaps");
    nightMapAtlas.registerAvailableTextures("textures/generated/nightmaps");
    heightMapAtlas.registerAvailableTextures("textures/generated/heightmaps");
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
    tileEarthRenderer->addSubscriber(cityNamesRenderer);
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
        return EXIT_FAILURE;
    }

    startRendering(renderers, guiRenderer, solarSimulator);
    cleanup(renderers);

    return EXIT_SUCCESS;
}

int main() {
    std::cout << "Starting the application..." << std::endl;
    std::cout << "Starting application thread: " << std::this_thread::get_id() << std::endl;

    std::thread loaderThread(resourceLoaderThreadStart);

    int returnCode = mainAppThread();

    stopThread = true;
    cv.notify_all();
    loaderThread.join();

    return returnCode;
}


