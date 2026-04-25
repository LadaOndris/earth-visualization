//
// Created by lada on 9/28/23.
//

#ifndef EARTH_VISUALIZATION_CAMERA_H
#define EARTH_VISUALIZATION_CAMERA_H

#include <glm/mat4x4.hpp> // glm::mat4

class Camera {
protected:
    float _fov;
    glm::vec3 _position;
    glm::vec3 _target;
public:
    explicit Camera(glm::vec3 position, glm::vec3 target, float fov = 45.0f)
            : _fov(fov), _position(position), _target(target) {
    }

    virtual ~Camera() = default;

    virtual void onMouseDrag(double xoffset, double yoffset) = 0;

    virtual void onMouseMove(double xoffset, double yoffset) = 0;

    virtual void onMouseScroll(double xoffset, double yoffset) = 0;

    [[nodiscard]] virtual glm::mat4 getViewMatrix() const = 0;

    [[nodiscard]] float getFov() const {
        return _fov;
    }

    [[nodiscard]] glm::vec3 getPosition() const {
        return _position;
    }

    [[nodiscard]] glm::vec3 getTarget() const {
        return _target;
    }

};

#endif //EARTH_VISUALIZATION_CAMERA_H
