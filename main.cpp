#include <iostream>
#include "include/glad/glad.h"
#include <GLFW/glfw3.h>
#include "include/shader.h"
#include <cmath>
#include <glm/vec3.hpp> // glm::vec3
#include <glm/vec4.hpp> // glm::vec4
#include <glm/mat4x4.hpp> // glm::mat4
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <utility>
#include <memory>
#include "include/camera.h"
#include "src/ellipsoid.h"
#include "src/tesselation/SubdivisionSphereTesselator.h"
#include "src/vertex.h"
#include "src/rendering/EarthRenderer.h"
#include "src/window_definition.h"

t_window_definition windowDefinition;
float lastX = 400, lastY = 300;
bool firstMouseMove = true;
GLFWwindow *window = nullptr;


Ellipsoid ellipsoid = Ellipsoid::unitSphere();
auto radii = ellipsoid.getRadii();
// From the side of the Earth
// Camera camera(5.0f, glm::vec3(-radii.x * 5, 0, 0));
// From above the Earth
Camera camera(5.0f, glm::vec3(0, radii.y * 5, 0), -90.f);

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
void initialize() {
    std::cout << "Starting the application..." << std::endl;

    glfwSetErrorCallback(error_callback);

    if (!glfwInit())
        exit(EXIT_FAILURE);

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    window = glfwCreateWindow(windowDefinition.width, windowDefinition.height, "Earth Viewer", NULL, NULL);
    if (!window) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    glfwMakeContextCurrent(window);
    if (!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        throw;
    }

    // The cursor should be captured and invisible
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glViewport(0, 0, windowDefinition.width, windowDefinition.height);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);
}

void startRendering(const std::vector<std::shared_ptr<Renderer>> &renderers) {
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

        glfwSwapBuffers(window);
        glfwPollEvents();
    }
}

void cleanup() {
    glfwDestroyWindow(window);
    glfwTerminate();
}

int main() {
    initialize();

    SubdivisionSphereTesselator subdivisionSurfaces;
    auto earthRenderer = std::make_shared<EarthRenderer>(ellipsoid, camera);
    earthRenderer->constructVertices(subdivisionSurfaces);
    earthRenderer->setupVertexArrays();
    earthRenderer->loadTextures();

    std::vector<std::shared_ptr<Renderer>> renderers;
    renderers.push_back(earthRenderer);

    startRendering(renderers);
    cleanup();

    exit(EXIT_SUCCESS);
}

