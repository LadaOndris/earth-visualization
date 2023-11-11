#version 400 core

layout (vertices=3) out;

in VS_OUT {
    vec3 geocentricFragPos;
    vec3 surfaceNormal;
} tc_in[];

out TC_OUT {
    vec3 geocentricFragPos;
    vec3 surfaceNormal;
} tc_out[];

float tesselationFactorOuter = 1.0;
float tesselationFactorInner = 64.0;

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

float getRawHeightDisplacement(vec3 geocentricCoordinates) {
    vec3 normal = convertGeocentricToGeocentricSurfaceNormal(geocentricCoordinates);
    vec2 globalTextureCoordinates = computeTextureCoordinates(normal);
    vec2 tileTextureCoordinates = calcTileHeightTextureCoordinates(globalTextureCoordinates);
    float rawDisplacement = texture(heightMapSampler, tileTextureCoordinates).r;
    return rawDisplacement;
}

void main()
{
    vec3 centroidPos =
        (tc_in[0].geocentricFragPos +
         tc_in[1].geocentricFragPos +
         tc_in[2].geocentricFragPos) / 3.0;

    float centroidHeight = getRawHeightDisplacement(centroidPos);
    float vertexHeight1 = getRawHeightDisplacement(tc_in[0].geocentricFragPos);
    float vertexHeight2 = getRawHeightDisplacement(tc_in[1].geocentricFragPos);
    float vertexHeight3 = getRawHeightDisplacement(tc_in[2].geocentricFragPos);

    float maxHeight = max(max(centroidHeight, vertexHeight1), max(vertexHeight2, vertexHeight3));
    float minHeight = min(min(centroidHeight, vertexHeight1), min(vertexHeight2, vertexHeight3));
    float slopeFactor = (maxHeight - minHeight);

    if (gl_InvocationID == 0) {
        float outerFactor = max(1, tesselationFactorOuter * slopeFactor);
        gl_TessLevelOuter[0] = outerFactor;
        gl_TessLevelOuter[1] = outerFactor;
        gl_TessLevelOuter[2] = outerFactor;

        float innerFactor = max(1, tesselationFactorInner * slopeFactor);
        gl_TessLevelInner[0] = innerFactor;
        gl_TessLevelInner[1] = innerFactor;
    }

    gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;
    // Copy input to output
    tc_out[gl_InvocationID].geocentricFragPos = tc_in[gl_InvocationID].geocentricFragPos;
    tc_out[gl_InvocationID].surfaceNormal = tc_in[gl_InvocationID].surfaceNormal;
}