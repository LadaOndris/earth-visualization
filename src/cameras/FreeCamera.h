
#pragma once

#include "printing.h"
#include "Camera.h"

class FreeCamera : public Camera {
public:
    explicit FreeCamera(float cameraSpeed, glm::vec3 cameraPos, float pitch = 0.f,
                        float fov = 45.0f) :
            Camera(cameraPos, glm::vec3(0, 0, 0), fov),
            _cameraSpeed(cameraSpeed), _cameraPos(cameraPos), _pitch(pitch) {
        assert(pitch <= 90 && pitch >= -90);
        updateDirection();
    }


    glm::mat4 getViewMatrix() const {
        glm::mat4 view = glm::lookAt(_cameraPos, _cameraPos + _cameraFront, _cameraUp);
        return view;
    }

    void moveUp(float deltaTime) {
        _cameraPos += deltaTime * _cameraSpeed * _cameraFront;
    }

    void moveDown(float deltaTime) {
        _cameraPos -= deltaTime * _cameraSpeed * _cameraFront;
    }

    void moveLeft(float deltaTime) {
        _cameraPos -= _cameraRight * deltaTime * _cameraSpeed;
    }

    void moveRight(float deltaTime) {
        _cameraPos += _cameraRight * deltaTime * _cameraSpeed;
    }

    void onMouseMove(double xoffset, double yoffset) {
        const float sensitivity = 0.1f;
        _yaw += static_cast<float>(xoffset) * sensitivity;
        _pitch += static_cast<float>(yoffset) * sensitivity;

        if (_pitch > 89.0f)
            _pitch = 89.0f;
        if (_pitch < -89.0f)
            _pitch = -89.0f;

        updateDirection();
    }

    void onMouseScroll(double xoffset, double yoffset) {
        _fov -= static_cast<float>(yoffset);
        if (_fov < 1.0f)
            _fov = 1.0f;
        if (_fov > 45.0f)
            _fov = 45.0f;
    }

    void onMouseDrag(double xoffset, double yoffset) {
        // No behaviour
    }
private:
    float _cameraSpeed;
    glm::vec3 _cameraPos = glm::vec3(-5.0f, 0.0f, 0.0f);
    float _pitch = 0.0f;

    float _yaw = 0.0f;
    glm::vec3 _worldUp = glm::vec3(0.0f, 1.0f, 0.0f);

    glm::vec3 _direction = glm::vec3(0.0f, 0.0f, 0.0f); // Is updated automatically
    glm::vec3 _cameraFront = glm::vec3(0.0f, 0.0f, 0.0f); // Is updated automatically
    glm::vec3 _cameraRight = glm::vec3(0.0f, 0.0f, 0.0f); // Is updated automatically
    glm::vec3 _cameraUp = glm::vec3(0.0f, 0.0f, 0.0f); // Is updated automatically

    void updateDirection() {
        _direction.x = glm::cos(glm::radians(_yaw)) * glm::cos(glm::radians(_pitch));
        _direction.y = glm::sin(glm::radians(_pitch));
        _direction.z = glm::sin(glm::radians(_yaw)) * glm::cos(glm::radians(_pitch));
        _cameraFront = glm::normalize(_direction);
        // normalize the vectors, because their length gets closer to 0 the more you look
        // up or down which results in slower movement.
        _cameraRight = glm::normalize(glm::cross(_cameraFront, _worldUp));
        _cameraUp = glm::normalize(glm::cross(_cameraRight, _cameraFront));
    }

};
