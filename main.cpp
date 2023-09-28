#include <iostream>
#include <cmath>
#include <utility>
#include <memory>

#include "include/glad/glad.h"
#include <GLFW/glfw3.h>

#include "include/shader.h"
#include "include/camera.h"
#include "src/ellipsoid.h"
#include "src/tesselation/SubdivisionSphereTesselator.h"
#include "src/vertex.h"
#include "src/rendering/EarthRenderer.h"
#include "src/window_definition.h"
#include "src/rendering/SunRenderer.h"

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

t_window_definition windowDefinition;
float lastX = 400, lastY = 300;
bool firstMouseMove = true;
GLFWwindow *window = nullptr;


Ellipsoid ellipsoid = Ellipsoid::unitSphere();
auto radii = ellipsoid.getRadii();
// From the side of the Earth
//Camera camera(5.0f, glm::vec3(-radii.x * 5, 0, 0),
//              0.f, 80.f);
Camera camera(5.0f, glm::vec3(-radii.x * 5, 0, 0),
              0, 55);
// From above the Earth
//Camera camera(5.0f, glm::vec3(0, radii.y * 5, 0), -90.f);

void error_callback(int error, const char *description) {
    fprintf(stderr, "Error: %s\n", description);
}

void framebuffer_size_callback(GLFWwindow *window, int width, int height) {
    std::cout << "Window resized to " + std::to_string(width) + "/" + std::to_string(height) << std::endl;
    glViewport(0, 0, width, height);
    windowDefinition.width = width;
    windowDefinition.height = height;
}

void mouse_callback(GLFWwindow *window, double xpos, double ypos) {
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

    camera.onMouseMove(xoffset, yoffset);
}

void scroll_callback(GLFWwindow *window, double xoffset, double yoffset) {
    camera.onMouseScroll(xoffset, yoffset);
}

void processInput(GLFWwindow *window, Camera &camera, float deltaTime) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.moveUp(deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.moveDown(deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.moveLeft(deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.moveRight(deltaTime);
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

    window = glfwCreateWindow(windowDefinition.width, windowDefinition.height, "Earth Viewer", NULL, NULL);
    if (!window) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    // The cursor should be captured and invisible
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);
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

std::string formatTime(const std::chrono::system_clock::time_point& timePoint) {
    // Convert time point to a time_t
    std::time_t time = std::chrono::system_clock::to_time_t(timePoint);

    // Convert time_t to a struct tm
    std::tm tmStruct = *std::localtime(&time);

    // Format the struct tm into a string
    char buffer[80];
    std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", &tmStruct);

    return buffer;
}

void renderImGuiFrame() {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();

    ImGui::NewFrame();

    // Window position
    ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_Always);
    // Window size
    ImGui::SetNextWindowSize(
            ImVec2(static_cast<float>(100), static_cast<float>(100)),
            ImGuiCond_Always
    );
    // Set window opacity
    ImGui::SetNextWindowBgAlpha(0.7f);

    ImGui::Begin("Controls", NULL, ImGuiWindowFlags_NoResize);

    ImGui::Dummy(ImVec2(0.0f, 1.0f));
    ImGui::TextColored(ImVec4(1.0f, 0.0f, 1.0f, 1.0f), "Time");
    ImGui::Text("%s", formatTime(std::chrono::system_clock::now()).c_str());

    ImGui::End();

    // Another window
    ImGui::Begin("Another Window");   // Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)
    ImGui::Text("Hello from another window!");
    if (ImGui::Button("Close Me")) {
        std::cout << "Button clicked!" << std::endl;
    }
    ImGui::End();

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void startRendering(const std::vector<std::shared_ptr<Renderer>> &renderers) {
    glViewport(0, 0, windowDefinition.width, windowDefinition.height);
    glEnable(GL_DEPTH_TEST);
    float lastFrame = 0.0f; // Time of last frame

    while (!glfwWindowShouldClose(window)) {
        auto currentFrame = static_cast<float>(glfwGetTime());
        float deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        processInput(window, camera, deltaTime);

        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        for (const auto &renderer: renderers) {
            renderer->render(currentFrame, windowDefinition);
        }
        renderImGuiFrame();

        glfwSwapBuffers(window);
        glfwPollEvents();
    }
}

void cleanup() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();
}

int main() {
    std::cout << "Starting the application..." << std::endl;
    if (!initializeGlfw() || !initializeGlad() || !initializeImgui()) {
        return EXIT_FAILURE;
    }

    auto sunVsEarthRadiusFactor = 109.168105; // Sun_radius / Earth_radius
    auto sunRadius = sunVsEarthRadiusFactor * radii.x;
    auto sunDistanceMeters = 149597870700.f;
    auto earthRadiusMeters = 6378000.f;
    auto sunDistance = sunDistanceMeters / earthRadiusMeters * radii.x;
    auto lightPosition = glm::vec3(0.0f, 0.0f, sunDistance);

    SubdivisionSphereTesselator subdivisionSurfaces;
    auto earthRenderer =
            std::make_shared<EarthRenderer>(ellipsoid, camera, lightPosition);
    earthRenderer->constructVertices(subdivisionSurfaces);
    earthRenderer->setupVertexArrays();
    earthRenderer->loadTextures();


    auto sunRenderer =
            std::make_shared<SunRenderer>(camera, lightPosition, sunRadius);
    sunRenderer->constructVertices();
    sunRenderer->setupVertexArrays();

    std::vector<std::shared_ptr<Renderer>> renderers;
    renderers.push_back(earthRenderer);
    renderers.push_back(sunRenderer);

    startRendering(renderers);
    cleanup();

    exit(EXIT_SUCCESS);
}

