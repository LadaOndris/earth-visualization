//
// Created by lada on 9/28/23.
//

#ifndef EARTH_VISUALIZATION_EARTHCENTEREDCAMERA_H
#define EARTH_VISUALIZATION_EARTHCENTEREDCAMERA_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include "Camera.h"
#include "../ellipsoid.h"

class EarthCenteredCamera : public Camera {

private:
    Ellipsoid &ellipsoid;
    glm::vec3 up;
    float sensitivityFactor = 0.09f;

    [[nodiscard]] float getDistanceToSurface() {
        glm::vec3 pointOnSurface = ellipsoid.projectGeocentricPointOntoSurface(position);
        float distance = glm::length(pointOnSurface - position);
        return distance;
    }

    float calcZoomSpeedFromDistance() {
        // Project camera onto surface to find out how far the camera from the surface is.
        auto distance = getDistanceToSurface();
        auto speed = distance / 10.f;
        //if (distance > 0.2f) {
        //auto speed = distance * distance / (distance + 30);
        //speed = std::min(speed, maxSpeed);
        //}
        //std::cout << "Distance: " << distance << " Speed: " << speed << std::endl;
        return speed;
    }

    float calcDragSensitivityBasedOnDistance() {
        auto distance = getDistanceToSurface();
        auto radiiLength = glm::length(ellipsoid.getRadii());
        // Normalize the distance by the radii of the Earth.
        float sensitivity = distance / radiiLength;
        return sensitivity * sensitivityFactor;
    }

public:
    EarthCenteredCamera(Ellipsoid &ellipsoid, glm::vec3 position, glm::vec3 target, glm::vec3 up,
                        float fov = 45.0f, float zoomSpeedFactor = 1.0f) :
            ellipsoid(ellipsoid), Camera(position, target, fov) {
        this->up = up;
    }

    glm::mat4 getViewMatrix() const {
        auto view = glm::lookAt(position, target, up);
        return view;
    }

    void onMouseScroll(double xoffset, double yoffset) {
        // Adjust the camera's position to zoom in or out
        float zoomSpeed = calcZoomSpeedFromDistance();
        auto positionShift = static_cast<float>(yoffset) * zoomSpeed * glm::normalize(target - position);
        auto positionCandidate = position + positionShift;

        if (ellipsoid.isPointOnTheOutside(positionCandidate)) {
            position = positionCandidate;
        }
    }

    void onMouseDrag(double xoffset, double yoffset) {
        // Calculate the rotation angles based on mouse movement
        float sensitivity = calcDragSensitivityBasedOnDistance();
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
