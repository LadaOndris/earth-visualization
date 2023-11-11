#version 400 core

layout (triangles) in;

in vec3 VertexPos[];

out vec3 geocentricFragPos;
out vec3 normal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

uniform bool isTerrainEnabled;
uniform sampler2D heightMapSampler;
uniform vec2 heightMapGeodeticOffset; // In radians
uniform vec2 heightMapGridSize;
uniform float heightDisplacementFactor;

uniform vec3 ellipsoidOneOverRadiiSquared;

const float PI = 3.14159265358979323846;
const float oneOverTwoPi = 1.0 / (2.0 * PI);
const float oneOverPi = 1.0 / PI;

vec3 convertGeocentricToGeocentricSurfaceNormal(vec3 point) {
    vec3 normal = point * ellipsoidOneOverRadiiSquared;
    return normalize(normal);
}

vec2 computeTextureCoordinates(vec3 normal)
{
    return vec2(
        atan(normal.z, normal.x) * oneOverTwoPi + 0.5,
        asin(normal.y) * oneOverPi + 0.5
    );
}

vec2 calcTileHeightTextureCoordinates(vec2 globalTextureCoordinates) {
    // Use the following direct approach to handle the problem with the poles
    float normalizedLongitude = (heightMapGeodeticOffset[0] + PI) / (2.0 * PI);
    float normalizedLatitude = (heightMapGeodeticOffset[1] + PI / 2.0) / PI;
    vec2 textureCoordinatesOffset = vec2(normalizedLongitude, normalizedLatitude);

    vec2 tileTextureCoordinates = (globalTextureCoordinates - textureCoordinatesOffset) * heightMapGridSize;
    return tileTextureCoordinates;
}

vec2 getHeightMapTextureCoords(vec3 geocentricCoordinates) {
    vec3 normal = convertGeocentricToGeocentricSurfaceNormal(geocentricCoordinates);
    vec2 globalTextureCoordinates = computeTextureCoordinates(normal);
    vec2 tileTextureCoordinates = calcTileHeightTextureCoordinates(globalTextureCoordinates);
    return tileTextureCoordinates;
}

float getHeightDisplacement(vec2 tileTextureCoordinates) {
    float rawDisplacement = texture(heightMapSampler, tileTextureCoordinates).r;
    float displacementFactor = 1 + rawDisplacement * heightDisplacementFactor;
    return displacementFactor;
}

vec3 computeNormalCentralDifference(vec2 position, sampler2D heightMap) {
    float upperLeft = texture(heightMap, position.xy + vec2(-1.0, 1.0)).r;
    float upperCenter = texture(heightMap, position.xy + vec2(0.0, 1.0)).r;
    float upperRight = texture(heightMap, position.xy + vec2(1.0, 1.0)).r;
    float left = texture(heightMap, position.xy + vec2(-1.0, 0.0)).r;
    float right = texture(heightMap, position.xy + vec2(1.0, 0.0)).r;
    float lowerLeft = texture(heightMap, position.xy + vec2(-1.0, -1.0)).r;
    float lowerCenter = texture(heightMap, position.xy + vec2(0.0, -1.0)).r;
    float lowerRight = texture(heightMap, position.xy + vec2(1.0, -1.0)).r;

    float x = upperRight + (2.0 * right) + lowerRight -
    upperLeft - (2.0 * left) - lowerLeft;
    float y = lowerLeft + (2.0 * lowerCenter) + lowerRight -
    upperLeft - (2.0 * upperCenter) - upperRight;

    return normalize(vec3(-x, y, 1.0));
}

void main()
{
    float u = gl_TessCoord.x;
    float v = gl_TessCoord.y;
    float w = gl_TessCoord.z;

    vec3 geocentricCoordinates = VertexPos[0] * u + VertexPos[1] * v + VertexPos[2] * w;

    vec2 heightMapTextureCoords = getHeightMapTextureCoords(geocentricCoordinates);

    if (isTerrainEnabled) {
        float heightDisplacement = getHeightDisplacement(heightMapTextureCoords);
        geocentricCoordinates *= heightDisplacement;
    }

    geocentricFragPos = geocentricCoordinates;
    normal = computeNormalCentralDifference(heightMapTextureCoords, heightMapSampler);

    gl_Position = projection * view * model * vec4(geocentricCoordinates, 1.0);
}