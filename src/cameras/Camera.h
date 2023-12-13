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
    glm::vec3 target;
public:
    explicit Camera(glm::vec3 position, glm::vec3 target, float fov = 45.0f)
            : fov(fov), position(position), target(target) {
    }

    virtual void onMouseDrag(double xoffset, double yoffset) = 0;

    virtual void onMouseMove(double xoffset, double yoffset) = 0;

    virtual void onMouseScroll(double xoffset, double yoffset) = 0;

    [[nodiscard]] virtual glm::mat4 getViewMatrix() const = 0;

    [[nodiscard]] float getFov() const {
        return fov;
    }

    [[nodiscard]] glm::vec3 getPosition() const {
        return position;
    }

    [[nodiscard]] glm::vec3 getTarget() const {
        return target;
    }

};

#endif //EARTH_VISUALIZATION_CAMERA_H
