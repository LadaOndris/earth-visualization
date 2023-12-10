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

    float slopeFactorEdge0 = abs(vertexHeight1 - vertexHeight2);
    float slopeFactorEdge1 = abs(vertexHeight1 - vertexHeight3);
    float slopeFactorEdge2 = abs(vertexHeight2 - vertexHeight3);

    float maxHeight = max(max(centroidHeight, vertexHeight1), max(vertexHeight2, vertexHeight3));
    float minHeight = min(min(centroidHeight, vertexHeight1), min(vertexHeight2, vertexHeight3));
    float slopeFactor = (maxHeight - minHeight);

    if (gl_InvocationID == 0) {
        // Calculate the screen space size of the triangle
        vec4 point1 = projection * view * model * vec4(tc_in[0].geocentricFragPos, 1);
        vec4 point2 = projection * view * model * vec4(tc_in[1].geocentricFragPos, 1);

        vec2 screenTriangleSize = abs(point1.xy / point1.w - point2.xy / point2.w);
        float screenSpaceSize = max(screenTriangleSize.x, screenTriangleSize.y);

        // Set a minimum screen space size threshold to avoid excessive tessellation
        float minScreenSpaceSize = 0.04; // Adjust this value as needed
        float maxTesselationFactor = screenSpaceSize / minScreenSpaceSize;

        gl_TessLevelOuter[0] = max(maxTesselationFactor, min(tesselationFactorOuter * slopeFactorEdge2, maxTesselationFactor));
        gl_TessLevelOuter[1] = max(maxTesselationFactor, min(tesselationFactorOuter * slopeFactorEdge1, maxTesselationFactor));
        gl_TessLevelOuter[2] = max(maxTesselationFactor, min(tesselationFactorOuter * slopeFactorEdge0, maxTesselationFactor));

        float innerFactor = max(maxTesselationFactor, min(tesselationFactorInner * slopeFactor, maxTesselationFactor));
        gl_TessLevelInner[0] = innerFactor;
    }

    gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;
    // Copy input to output
    tc_out[gl_InvocationID].geocentricFragPos = tc_in[gl_InvocationID].geocentricFragPos;
    tc_out[gl_InvocationID].surfaceNormal = tc_in[gl_InvocationID].surfaceNormal;
}