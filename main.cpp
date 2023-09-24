#include <iostream>
#include "include/glad/glad.h"
#include <GLFW/glfw3.h>
#include "include/shader.h"
#include "include/stb_image.h"
#include <cmath>
#include <glm/vec3.hpp> // glm::vec3
#include <glm/vec4.hpp> // glm::vec4
#include <glm/mat4x4.hpp> // glm::mat4
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "include/camera.h"
#include "src/ellipsoid.h"
#include "src/tesselation/SubdivisionSurfaces.h"

class OpenGLState {
public:
    unsigned int VAO;
};

int WINDOW_WIDTH = 640;
int WINDOW_HEIGHT = 480;
float lastX = 400, lastY = 300;
bool firstMouseMove = true;
Camera camera(2.5f);

Ellipsoid ellipsoid(1, 1, 1);
SubdivisionSurfaces subdivisionSurfaces(ellipsoid);

float texCoords[] = {
        0.0f, 0.0f,  // lower-left corner
        1.0f, 0.0f,  // lower-right corner
        0.5f, 1.0f   // top-center corner
};

void error_callback(int error, const char *description) {
    fprintf(stderr, "Error: %s\n", description);
}

void framebuffer_size_callback(GLFWwindow *window, int width, int height) {
    std::cout << "Window resized to " + std::to_string(width) + "/" + std::to_string(height) << std::endl;
    glViewport(0, 0, width, height);
    WINDOW_WIDTH = width;
    WINDOW_HEIGHT = height;
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

typedef struct {
    float x, y, z;
} t_vertex;

std::vector<t_vertex> convertToVertices(std::vector<glm::vec3> vertexVecs) {
    std::vector<t_vertex> vertices;
    for (const auto &vec3: vertexVecs) {
        t_vertex vertex;
        vertex.x = vec3.x;
        vertex.y = vec3.y;
        vertex.z = vec3.z;
        vertices.push_back(vertex);
    }

    return vertices;
}

void setup_gl(OpenGLState &state, std::vector<t_vertex> vertices) {
    unsigned int VBO;
    glGenBuffers(1, &VBO);

    glGenVertexArrays(1, &state.VAO);
    glBindVertexArray(state.VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(t_vertex) , &vertices.front(), GL_STATIC_DRAW);

//    unsigned int EBO;
//    glGenBuffers(1, &EBO);
//    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
//    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // Position
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *) 0);
    glEnableVertexAttribArray(0);
    // Color
//    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *) (3 * sizeof(float)));
//    glEnableVertexAttribArray(1);
    // Texture
//    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *) (3 * sizeof(float)));
//    glEnableVertexAttribArray(2);
}


int main() {
    std::cout << "Starting the application..." << std::endl;

    std::cout << "Generating triangles..." << std::endl;

    auto verticesVecs = subdivisionSurfaces.tessellate(6);
    std::vector<t_vertex> vertices = convertToVertices(verticesVecs);

    std::cout << "Generated " + std::to_string(vertices.size() / 3) + " triangles." << std::endl;

    glfwSetErrorCallback(error_callback);

    if (!glfwInit())
        exit(EXIT_FAILURE);

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow *window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Earth Viewer", NULL, NULL);
    if (!window) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    glfwMakeContextCurrent(window);
    if (!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // The cursor should be captured and invisible
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);

    Shader shader("shaders/shader.vs", "shaders/shader.fs");
    auto gl_state = OpenGLState();
    setup_gl(gl_state, vertices);

    // Create texture
    unsigned int texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    int width, height, nrChannels;
    unsigned char *data = stbi_load("textures/container.jpg", &width, &height, &nrChannels, 0);
    if (data) {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    } else {
        std::cout << "Failed to load texture" << std::endl;
    }
    stbi_image_free(data);

    glEnable(GL_DEPTH_TEST);
    float lastFrame = 0.0f; // Time of last frame

    while (!glfwWindowShouldClose(window)) {
        float currentFrame = glfwGetTime();
        float deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        processInput(window, camera, deltaTime);

        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glBindTexture(GL_TEXTURE_2D, texture);

        glm::mat4 projectionMatrix;
        projectionMatrix = glm::perspective(glm::radians(camera.getFov()), WINDOW_WIDTH / (float)WINDOW_HEIGHT, 0.1f, 100.0f);

        shader.use();

        shader.setVec3("objectColor", glm::vec3(1.0f, 0.5f, 0.31f));
        shader.setVec3("lightColor",  glm::vec3(1.0f, 1.0f, 1.0f));
        shader.setVec3("lightPos", glm::vec3(1.0f, 2.0f, 3.0f));

        // note: currently we set the projection matrix each frame, but since the projection matrix rarely changes
        // it's often best practice to set it outside the main loop only once.
        shader.setMat4("projection", projectionMatrix);
        shader.setMat4("view", camera.getViewMatrix());

        glBindVertexArray(gl_state.VAO);
        glm::mat4 modelMatrix = glm::mat4(1.0f);
        glm::vec3 ellipsoidPosition = glm::vec3(0.f, 0.f, 0.f);
        modelMatrix = glm::translate(modelMatrix, ellipsoidPosition);
        float angle = 20.0f * 0;
        modelMatrix = glm::rotate(modelMatrix, (float) glfwGetTime() * glm::radians(angle),
                                  glm::vec3(1.0f, 0.3f, 0.5f));
        shader.setMat4("model", modelMatrix);

        glDrawArrays(GL_TRIANGLES, 0, vertices.size());

        // glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwDestroyWindow(window);
    glfwTerminate();
    exit(EXIT_SUCCESS);
}

