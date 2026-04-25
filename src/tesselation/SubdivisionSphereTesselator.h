#pragma once

#include <glm/fwd.hpp>

#include <vector>

class SphereTesselator {
public:
    virtual ~SphereTesselator() = default;

    virtual std::vector<glm::vec3> tessellate(int repetitions) = 0;
};

class SubdivisionSphereTesselator : public SphereTesselator {
public:
    explicit SubdivisionSphereTesselator();

    std::vector<glm::vec3> tessellate(int repetitions) override;

private:
    std::vector<glm::vec3> vertices;
};
