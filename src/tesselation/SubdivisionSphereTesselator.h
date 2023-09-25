//
// Created by lada on 9/24/23.
//

#ifndef EARTH_VISUALIZATION_SUBDIVISIONSPHERETESSELATOR_H
#define EARTH_VISUALIZATION_SUBDIVISIONSPHERETESSELATOR_H


#include <vector>
#include "../ellipsoid.h"

class SubdivisionSphereTesselator {
public:
    explicit SubdivisionSphereTesselator();
    std::vector<glm::vec3> tessellate(int repetitions);
private:
    std::vector<glm::vec3> vertices;
};


#endif //EARTH_VISUALIZATION_SUBDIVISIONSPHERETESSELATOR_H
