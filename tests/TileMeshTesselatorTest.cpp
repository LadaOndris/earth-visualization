

#include <memory>
#include "gtest/gtest.h"
#include "../src/tesselation/TileMeshTesselator.h"
#include <glm/mat4x4.hpp> // glm::mat4

class TileMeshTesselatorFixture : public ::testing::Test {
protected:
    virtual void SetUp() {
        tesselator = std::make_unique<TileMeshTesselator>();
    }

    std::unique_ptr<TileMeshTesselator> tesselator;
};

TEST_F(TileMeshTesselatorFixture, GeneratesTheCorrectResolution) {
    Resolution resolution(5, 4);
    Ellipsoid ellipsoid = Ellipsoid::unitSphere();
    Tile tile(0, 90, 10, 10);

    Mesh_t mesh = tesselator->generate(resolution, ellipsoid, tile);

    // Each square is made up of two triangles.
    int numTriangles = (resolution.getWidth() - 1) * (resolution.getHeight() - 1) * 2;
    int numVertices = numTriangles * 3;

    ASSERT_EQ(mesh.size(), numVertices);
}

TEST_F(TileMeshTesselatorFixture, PointsLieOnUnitSphere) {
    Resolution resolution(5, 4);
    Ellipsoid ellipsoid = Ellipsoid::unitSphere();
    Tile tile(0, 90, 10, 10);

    Mesh_t mesh = tesselator->generate(resolution, ellipsoid, tile);

    float expectedLength = 1.f;
    for (auto &vertex : mesh) {
        EXPECT_NEAR(glm::length(vertex), expectedLength, 0.00001);
    }
}