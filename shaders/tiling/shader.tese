#version 400 core

layout(triangles) in;

in vec3 VertexPos[];

out vec3 geocentricFragPos;

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

float getHeightDisplacement(vec3 geocentricCoordinates) {
    vec3 normal = convertGeocentricToGeocentricSurfaceNormal(geocentricCoordinates);
    vec2 globalTextureCoordinates = computeTextureCoordinates(normal);
    vec2 tileTextureCoordinates = calcTileHeightTextureCoordinates(globalTextureCoordinates);
    float rawDisplacement = texture(heightMapSampler, tileTextureCoordinates).r;
    float displacementFactor = 1 + rawDisplacement * heightDisplacementFactor;
    return displacementFactor;
}

void main()
{
    float u = gl_TessCoord.x;
    float v = gl_TessCoord.y;
    float w = gl_TessCoord.z;

    vec3 geocentricCoordinates = VertexPos[0] * u + VertexPos[1] * v + VertexPos[2] * w;

    if (isTerrainEnabled) {
        float heightDisplacement = getHeightDisplacement(geocentricCoordinates);
        geocentricCoordinates *= heightDisplacement;
    }

    geocentricFragPos = geocentricCoordinates;

    gl_Position = projection * view * model * vec4(geocentricCoordinates, 1.0);
}