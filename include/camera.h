
#ifndef EARTH_VISUALIZATION_CAMERA_H
#define EARTH_VISUALIZATION_CAMERA_H

#include "../include/printing.h"

class Camera {
private:
    float cameraSpeed;
    glm::vec3 cameraPos = glm::vec3(-5.0f, 0.0f, 0.0f);
    glm::vec3 worldUp = glm::vec3(0.0f, 1.0f, 0.0f);

    glm::vec3 direction = glm::vec3(0.0f, 0.0f, 0.0f); // Is updated automatically
    glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, 0.0f); // Is updated automatically
    glm::vec3 cameraRight = glm::vec3(0.0f, 0.0f, 0.0f); // Is updated automatically
    glm::vec3 cameraUp = glm::vec3(0.0f, 0.0f, 0.0f); // Is updated automatically

    float yaw = 0.0f;
    float pitch = 0.0f;
    float fov = 45.0f;

    void updateDirection() {
        direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
        direction.y = sin(glm::radians(pitch));
        direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
        cameraFront = glm::normalize(direction);
        // normalize the vectors, because their length gets closer to 0 the more you look
        // up or down which results in slower movement.
        cameraRight = glm::normalize(glm::cross(cameraFront, worldUp));
        cameraUp = glm::normalize(glm::cross(cameraRight, cameraFront));
    }

public:
    explicit Camera(float cameraSpeed, glm::vec3 cameraPos, float pitch = 0.f,
                    float fov = 45.0f) :
            cameraSpeed(cameraSpeed), cameraPos(cameraPos), pitch(pitch),
            fov(fov) {
        assert(pitch <= 90 && pitch >= -90);
        updateDirection();
    }


    glm::mat4 getViewMatrix() const {
        glm::mat4 view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
        return view;
    }

    float getFov() const {
        return fov;
    }

    void moveUp(float deltaTime) {
        cameraPos += deltaTime * cameraSpeed * cameraFront;
    }

    void moveDown(float deltaTime) {
        cameraPos -= deltaTime * cameraSpeed * cameraFront;
    }

    void moveLeft(float deltaTime) {
        cameraPos -= cameraRight * deltaTime * cameraSpeed;
    }

    void moveRight(float deltaTime) {
        cameraPos += cameraRight * deltaTime * cameraSpeed;
    }

    void onMouseMove(double xoffset, double yoffset) {
        const float sensitivity = 0.1f;
        xoffset *= sensitivity;
        yoffset *= sensitivity;

        yaw += xoffset;
        pitch += yoffset;

        if (pitch > 89.0f)
            pitch = 89.0f;
        if (pitch < -89.0f)
            pitch = -89.0f;

        updateDirection();
    }

    void onMouseScroll(double xoffset, double yoffset) {
        fov -= (float) yoffset;
        if (fov < 1.0f)
            fov = 1.0f;
        if (fov > 45.0f)
            fov = 45.0f;
    }
};

#endif
