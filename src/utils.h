
#ifndef EARTH_VISUALIZATION_UTILS_H
#define EARTH_VISUALIZATION_UTILS_H

#include <cmath>
#include <glm/vec3.hpp>

namespace utils {

    static const float TO_RADS_COEFF = static_cast<float>(M_PI / 180.0);

    template<typename T>
    T convertToRads(T vector) {
        return vector * TO_RADS_COEFF;
    }

}

#endif //EARTH_VISUALIZATION_UTILS_H
