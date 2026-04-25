
#include "glad/glad.h"
#include <GLFW/glfw3.h>

#include "src/EarthVisualizer.h"
#include "src/window_definition.h"

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include <unistd.h>
#include <thread>
#include <iostream>

t_window_definition gWindowDefinition{800, 600};
float gLastX = 400, gLastY = 300;
bool gFirstMouseMove = true;
bool gLbuttonDown = false;
GLFWwindow *gWindow = nullptr;


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
}

void cursor_pos_callback(GLFWwindow *window, double xpos, double ypos) {
    auto &io = ImGui::GetIO();
    if (io.WantCaptureMouse) {
        return;
    }

    if (gFirstMouseMove) {
        gLastX = xpos;
        gLastY = ypos;
        gFirstMouseMove = false;
    }

    float xoffset = static_cast<float>(xpos) - gLastX;
    float yoffset = gLastY - static_cast<float>(ypos);
    gLastX = static_cast<float>(xpos);
    gLastY = static_cast<float>(ypos);

    auto& camera = static_cast<EarthVisualizer*>(glfwGetWindowUserPointer(window))->getCamera();
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

    static_cast<EarthVisualizer*>(glfwGetWindowUserPointer(window))->getCamera()
        .onMouseScroll(xoffset, yoffset);
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
        auto& camera = static_cast<EarthVisualizer*>(glfwGetWindowUserPointer(window))->getCamera();
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

void initializeGlfw() {
    glfwSetErrorCallback(error_callback);

    if (!glfwInit())
        throw std::runtime_error("[ERROR] Couldn't initialize GLFW");

    if (!getenv("DISPLAY") && !getenv("WAYLAND_DISPLAY")) {
        glfwTerminate();
        throw std::runtime_error(
            "[ERROR] No display server detected (DISPLAY or WAYLAND_DISPLAY not set). "
            "This application requires a GUI environment to run."
        );
    }

    std::cout << "[INFO] GLFW initialized" << std::endl;

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
        glfwTerminate();
        throw std::runtime_error("Failed to create GLFW window");
    }

    glfwSetFramebufferSizeCallback(gWindow, framebuffer_size_callback);
    glfwSetCursorPosCallback(gWindow, cursor_pos_callback);
    glfwSetMouseButtonCallback(gWindow, mouse_button_callback);
    glfwSetScrollCallback(gWindow, scroll_callback);
    glfwSetKeyCallback(gWindow, key_callback);
}

void initializeGlad() {
    if (!gWindow)
        throw std::runtime_error("[ERROR] window is null, cannot initialize GLAD");

    glfwMakeContextCurrent(gWindow);
    if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress)))
        throw std::runtime_error("[ERROR] Failed to initialize GLAD");

    std::cout << "[INFO] GLAD initialized" << std::endl;
}

void initializeImgui() {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    ImGuiIO &io = ImGui::GetIO();
    (void) io;

    if (!ImGui_ImplGlfw_InitForOpenGL(gWindow, true))
        throw std::runtime_error("[ERROR] Failed to initialize ImGui (ImGui_ImplGlfw_InitForOpenGL)");

    if (!ImGui_ImplOpenGL3_Init())
        throw std::runtime_error("[ERROR] Failed to initialize ImGui (ImGui_ImplOpenGL3_Init)");

    std::cout << "[INFO] IMGUI initialized" << std::endl;
}

void cleanup() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(gWindow);
    glfwTerminate();
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
    try {
        initializeGlfw();
        initializeGlad();
        initializeImgui();
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    if (glDebugMessageCallback) {
        glDebugMessageCallback(processErrorMessageCallback, nullptr);
        glEnable(GL_DEBUG_OUTPUT);
    }

    try {
        EarthVisualizer visualizer(gWindow, gWindowDefinition);
        glfwSetWindowUserPointer(gWindow, &visualizer);
        visualizer.run();
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        cleanup();
        return EXIT_FAILURE;
    }

    cleanup();
    return EXIT_SUCCESS;
}

int main() {
    std::cout << "Starting the application..." << std::endl;
    std::cout << "Starting application thread: " << std::this_thread::get_id() << std::endl;

    return mainAppThread();
}
