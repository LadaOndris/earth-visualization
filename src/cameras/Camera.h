//
// Created by lada on 9/28/23.
//

#ifndef EARTH_VISUALIZATION_CAMERA_H
#define EARTH_VISUALIZATION_CAMERA_H

#include <glm/mat4x4.hpp> // glm::mat4

class Camera {
protected:
    float fov;
    glm::vec3 position;
public:
    explicit Camera(float fov = 45.0f) : fov(fov) {
    }

    virtual void onMouseDrag(double xoffset, double yoffset) = 0;

    virtual void onMouseMove(double xoffset, double yoffset) = 0;

    virtual void onMouseScroll(double xoffset, double yoffset) = 0;

    virtual glm::mat4 getViewMatrix() const = 0;

    float getFov() const {
        return fov;
    }

    glm::vec3 getPosition() const {
        return position;
    }
};

#endif //EARTH_VISUALIZATION_CAMERA_H
