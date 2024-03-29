#version 400 core

layout (triangles) in;

in TC_OUT {
    vec3 geocentricFragPos;
    vec3 surfaceNormal;
} te_in[];

out TE_OUT {
    vec3 geocentricFragPos;
    vec3 surfaceNormal;
} te_out;

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

void main()
{
    float u = gl_TessCoord.x;
    float v = gl_TessCoord.y;
    float w = gl_TessCoord.z;

    vec3 geocentricCoordinates =
    te_in[0].geocentricFragPos * u +
    te_in[1].geocentricFragPos * v +
    te_in[2].geocentricFragPos * w;

    vec3 surfaceNormal =
    te_in[0].surfaceNormal * u +
    te_in[1].surfaceNormal * v +
    te_in[2].surfaceNormal * w;

    if (isTerrainEnabled) {
        vec2 heightMapTextureCoords = getHeightMapTextureCoords(geocentricCoordinates);
        float heightDisplacement = getHeightDisplacement(heightMapTextureCoords);
        geocentricCoordinates *= heightDisplacement;
    }

    te_out.geocentricFragPos = geocentricCoordinates;
    te_out.surfaceNormal = surfaceNormal;

    gl_Position = projection * view * model * vec4(geocentricCoordinates, 1.0);
}