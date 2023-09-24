//
// Created by lada on 9/24/23.
//

#ifndef EARTH_VISUALIZATION_SUBDIVISIONSURFACES_H
#define EARTH_VISUALIZATION_SUBDIVISIONSURFACES_H


#include <vector>
#include "../ellipsoid.h"

class SubdivisionSurfaces {
public:
    explicit SubdivisionSurfaces(Ellipsoid &ellipsoid);
    std::vector<glm::vec3> tessellate(int repetitions);
private:
    Ellipsoid &ellipsoid;
    std::vector<glm::vec3> vertices;
};


#endif //EARTH_VISUALIZATION_SUBDIVISIONSURFACES_H
