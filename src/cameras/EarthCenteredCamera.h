//
// Created by lada on 9/28/23.
//

#ifndef EARTH_VISUALIZATION_EARTHCENTEREDCAMERA_H
#define EARTH_VISUALIZATION_EARTHCENTEREDCAMERA_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "Camera.h"

class EarthCenteredCamera : public Camera {

private:
    glm::vec3 position;
    glm::vec3 target;
    glm::vec3 up;

public:
    EarthCenteredCamera(glm::vec3 position, glm::vec3 target, glm::vec3 up,
                        float fov = 45.0f) : Camera(fov) {
        this->position = position;
        this->target = target;
        this->up = up;
    }

    glm::mat4 getViewMatrix() const {
        auto view = glm::lookAt(position, target, up);
        return view;
    }

    void onMouseScroll(double xoffset, double yoffset) {
        // Adjust the camera's position to zoom in or out
        float zoomSpeed = 0.1f;
        position += static_cast<float>(yoffset) * zoomSpeed * glm::normalize(target - position);
    }

    void onMouseDrag(double xoffset, double yoffset) {
        // Calculate the rotation angles based on mouse movement
        float sensitivity = 0.1f;
        float yaw = -static_cast<float>(xoffset) * sensitivity;
        float pitch = -static_cast<float>(yoffset) * sensitivity;

        // Rotate the camera's position around the target point
        glm::vec3 toTarget = target - position;
        glm::mat4 rotation = glm::mat4(1.0f);
        rotation = glm::rotate(rotation, glm::radians(yaw), up);
        rotation = glm::rotate(rotation, glm::radians(-pitch), glm::cross(toTarget, up));
        toTarget = glm::vec3(rotation * glm::vec4(toTarget, 0.0f));

        // Update the camera's position
        position = target - toTarget;
    }

    void onMouseMove(double xoffset, double yoffset) {
        // No behaviour
    }

};


#endif //EARTH_VISUALIZATION_EARTHCENTEREDCAMERA_H
