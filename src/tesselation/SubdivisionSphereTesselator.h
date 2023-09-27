//
// Created by lada on 9/24/23.
//

#ifndef EARTH_VISUALIZATION_SUBDIVISIONSPHERETESSELATOR_H
#define EARTH_VISUALIZATION_SUBDIVISIONSPHERETESSELATOR_H


#include <vector>
#include "../ellipsoid.h"

class SphereTesselator {
public:
    virtual std::vector<glm::vec3> tessellate(int repetitions) = 0;
};

class SubdivisionSphereTesselator : public SphereTesselator {
public:
    explicit SubdivisionSphereTesselator();

    std::vector<glm::vec3> tessellate(int repetitions) override;

private:
    std::vector<glm::vec3> vertices;
};


#endif //EARTH_VISUALIZATION_SUBDIVISIONSPHERETESSELATOR_H
