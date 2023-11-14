//
// Created by lada on 11/4/23.
//

#ifndef EARTH_VISUALIZATION_LIGHTSOURCE_H
#define EARTH_VISUALIZATION_LIGHTSOURCE_H


#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>

class LightSource {

public:
    [[nodiscard]] virtual glm::vec3 getLightPosition() const = 0;

    [[nodiscard]] virtual glm::mat4 getTransformationMatrix() const = 0;
};


#endif //EARTH_VISUALIZATION_LIGHTSOURCE_H
