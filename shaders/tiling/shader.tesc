#version 400 core

layout (vertices = 3) out;

in VS_OUT {
    vec3 geocentricFragPos;
    vec3 surfaceNormal;
} tc_in[];

out TC_OUT {
    vec3 geocentricFragPos;
    vec3 surfaceNormal;
} tc_out[];

float tesselationFactorOuter = 64.0;
float tesselationFactorInner = 64.0;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

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

float calculateBaseTesselationFactor(vec2 ndcPoint1, vec2 ndcPoint2) {
    vec2 screenTriangleSize = abs(ndcPoint1 - ndcPoint2);
    float screenSpaceSize = max(screenTriangleSize.x, screenTriangleSize.y);

    // Set a minimum screen space size threshold to avoid excessive tessellation
    float minScreenSpaceSize = 0.04; // Adjust this value as needed
    float tesselationFactor = screenSpaceSize / minScreenSpaceSize;
    return tesselationFactor;
}

void main()
{
    vec3 centroidPos =
    (tc_in[0].geocentricFragPos +
    tc_in[1].geocentricFragPos +
    tc_in[2].geocentricFragPos) / 3.0;

    float vertexHeight[3];
    float slopeFactorEdge[3]; /* Contains slopes along edges of a triangle */
    float baseTesselationFactor[3]; /* Default tesselation factor calculated based on screen space size */

    float centroidHeight = getRawHeightDisplacement(centroidPos);
    for (int i = 0; i < 3; ++i) {
        vertexHeight[i] = getRawHeightDisplacement(tc_in[i].geocentricFragPos);
    }

    slopeFactorEdge[0] = abs(vertexHeight[0] - vertexHeight[1]);
    slopeFactorEdge[1] = abs(vertexHeight[0] - vertexHeight[2]);
    slopeFactorEdge[2] = abs(vertexHeight[1] - vertexHeight[2]);

    float maxHeight = max(max(centroidHeight, vertexHeight[0]), max(vertexHeight[1], vertexHeight[2]));
    float minHeight = min(min(centroidHeight, vertexHeight[0]), min(vertexHeight[1], vertexHeight[2]));
    float slopeFactor = (maxHeight - minHeight);

    if (gl_InvocationID == 0) {
        // Calculate the screen space size of the triangle
        vec2 ndcPoints[3];
        for (int i = 0; i < 3; ++i) {
            vec4 point = projection * view * model * vec4(tc_in[i].geocentricFragPos, 1);
            ndcPoints[i] = point.xy / point.w;
        }

        // Tesselation factors calculated baed on screen space size
        baseTesselationFactor[0] = calculateBaseTesselationFactor(ndcPoints[0], ndcPoints[1]);
        baseTesselationFactor[1] = calculateBaseTesselationFactor(ndcPoints[0], ndcPoints[2]);
        baseTesselationFactor[2] = calculateBaseTesselationFactor(ndcPoints[1], ndcPoints[2]);

        int factor = 2;
        for (int i = 0; i < 3; ++i) {
            // Use tesselation as determined by the height map. If there is no height, use at least the baseTesselation
            // as determined by the screen space size. Limit the tesselation factor as determined by the height
            // to a factor of the baseTesselation.
            gl_TessLevelOuter[i] = min(baseTesselationFactor[2 - i] * factor, max(tesselationFactorOuter * slopeFactorEdge[2 - i], baseTesselationFactor[2 - i]));
        }

        float innerFactor = min(baseTesselationFactor[0] * factor, max(tesselationFactorInner * slopeFactor, baseTesselationFactor[0]));
        gl_TessLevelInner[0] = innerFactor;
    }

    gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;
    // Copy input to output
    tc_out[gl_InvocationID].geocentricFragPos = tc_in[gl_InvocationID].geocentricFragPos;
    tc_out[gl_InvocationID].surfaceNormal = tc_in[gl_InvocationID].surfaceNormal;
}