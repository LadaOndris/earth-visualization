
#pragma once

#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>

class LightSource {
public:
    virtual ~LightSource() = default;

    [[nodiscard]] virtual glm::vec3 getLightPosition() const = 0;

    [[nodiscard]] virtual glm::mat4 getTransformationMatrix() const = 0;
};
