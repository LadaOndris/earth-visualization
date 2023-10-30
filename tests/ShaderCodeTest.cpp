
#include <memory>
#include "gtest/gtest.h"
#include "../src/textures/TextureAtlas.h"

using namespace glm;

class TextureAtlasFixture : public ::testing::Test {
protected:
    virtual void SetUp() {
    }

};

vec2 computeTextureCoordinates(vec3 normal) {
    const float PI = 3.14159265358979323846;
    float oneOverTwoPi = 1.0 / (2.0 * PI);
    float oneOverPi = 1.0 / PI;

    // TODO: What if normal.x == 0?
    return vec2(
            atan2(normal.z, normal.x) * oneOverTwoPi + 0.5,
            asin(normal.y) * oneOverPi + 0.5
    );
}

TEST_F(TextureAtlasFixture, CorrectlyConvertsCoordiantes) {
    float TO_RADS_COEFF = static_cast<float>(M_PI / 180.0);

    Ellipsoid ellipsoid = Ellipsoid::unitSphere();

    // Texture will be 90 degrees in size in long and lat
    vec2 textureGridSize(4, 2);
    vec2 textureGeodeticOffset(0, 0); // Because it starts from (-180, -90)
    textureGeodeticOffset = textureGeodeticOffset * TO_RADS_COEFF;

    vec2 aPos(0.5, 0.5); // In the middle of the tile
    float uTileLongitudeOffset = 36; // This is texture long=3
    float uTileLatitudeOffset = 18; // This is texture lat=1
    float uTileLongitudeWidth = 9;
    float uTileLatitudeWidth = 9;

    float longitude = uTileLongitudeOffset + aPos.x * uTileLongitudeWidth;
    float latitude = uTileLatitudeOffset + aPos.y * uTileLatitudeWidth;
    longitude = longitude * TO_RADS_COEFF;
    latitude = latitude * TO_RADS_COEFF;

    vec3 geographicCoordinates = vec3(longitude, latitude, 0.0);

    vec3 geocentricFragPos = ellipsoid.convertGeodeticToGeocentric(geographicCoordinates);
    vec3 normal = ellipsoid.convertGeocentricToGeocentricSurfaceNormal(geocentricFragPos);
    vec2 globalTextureCoordinates = computeTextureCoordinates(normal);

    vec3 textureGeocentricOffset = ellipsoid.convertGeodeticToGeocentric(
            vec3(textureGeodeticOffset[0], textureGeodeticOffset[1], 0));
    vec3 textureOffsetNormal = ellipsoid.convertGeocentricToGeocentricSurfaceNormal(textureGeocentricOffset);
    vec2 textureCoordinatesOffset = computeTextureCoordinates(textureOffsetNormal);

    vec2 tileTextureCoordinates = (globalTextureCoordinates - textureCoordinatesOffset) * textureGridSize;

    // Texture coords are in the [0, 1] range
    // The tex coords of the tile's longitude is in [0.4, 0.5] ([36/90, (36+9)/90]),
    // and latitude [0.2, 0.3] ([18/90, (18+9)/90]).

    EXPECT_FLOAT_EQ(tileTextureCoordinates[0], 0.45);
    EXPECT_FLOAT_EQ(tileTextureCoordinates[1], 0.25);
}


TEST_F(TextureAtlasFixture, CorrectlyConvertsCoordinates2) {
    float TO_RADS_COEFF = static_cast<float>(M_PI / 180.0);

    Ellipsoid ellipsoid = Ellipsoid::unitSphere();

    // Texture will be 90 degrees in size in long and lat
    vec2 textureGridSize(4, 2);
    vec2 textureGeodeticOffset(90, 45); // It starts from (-180, -90)
    textureGeodeticOffset = textureGeodeticOffset * TO_RADS_COEFF;

    vec2 aPos(1., 0.);
    float uTileLongitudeOffset = 90 + 36; // This is texture long=3
    float uTileLatitudeOffset = 45 + 18; // This is texture lat=1
    float uTileLongitudeWidth = 9;
    float uTileLatitudeWidth = 9;

    float longitude = uTileLongitudeOffset + aPos.x * uTileLongitudeWidth;
    float latitude = uTileLatitudeOffset + aPos.y * uTileLatitudeWidth;
    longitude = longitude * TO_RADS_COEFF;
    latitude = latitude * TO_RADS_COEFF;

    vec3 geographicCoordinates = vec3(longitude, latitude, 0.0);

    vec3 geocentricFragPos = ellipsoid.convertGeodeticToGeocentric(geographicCoordinates);
    vec3 normal = ellipsoid.convertGeocentricToGeocentricSurfaceNormal(geocentricFragPos);
    vec2 globalTextureCoordinates = computeTextureCoordinates(normal);

    vec3 textureGeocentricOffset = ellipsoid.convertGeodeticToGeocentric(
            vec3(textureGeodeticOffset[0], textureGeodeticOffset[1], 0));
    vec3 textureOffsetNormal = ellipsoid.convertGeocentricToGeocentricSurfaceNormal(textureGeocentricOffset);
    vec2 textureCoordinatesOffset = computeTextureCoordinates(textureOffsetNormal);

    vec2 tileTextureCoordinates = (globalTextureCoordinates - textureCoordinatesOffset) * textureGridSize;

    EXPECT_FLOAT_EQ(tileTextureCoordinates[0], 0.5);
    EXPECT_FLOAT_EQ(tileTextureCoordinates[1], 0.2);
}
