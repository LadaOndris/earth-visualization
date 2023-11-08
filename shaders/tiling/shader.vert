#version 330 core
// Offset of the vertex within the tile in longitude and latitude
// in the [0, 1] range.
layout (location = 0) in vec3 aPos;

out vec3 vertexColor;
out vec3 geocentricFragPos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

uniform vec3 ellipsoidRadiiSquared;
uniform vec3 ellipsoidOneOverRadiiSquared;

uniform float uTileLongitudeOffset;
uniform float uTileLatitudeOffset;
uniform float uTileLongitudeWidth;
uniform float uTileLatitudeWidth;

uniform bool isTerrainEnabled;
uniform sampler2D heightMapSampler;
uniform vec2 heightMapGeodeticOffset; // In radians
uniform vec2 heightMapGridSize;
uniform float heightDisplacementFactor;

const float PI = 3.14159265358979323846;
const float oneOverTwoPi = 1.0 / (2.0 * PI);
const float oneOverPi = 1.0 / PI;

vec3 convertGeocentricToGeocentricSurfaceNormal(vec3 point) {
    vec3 normal = point * ellipsoidOneOverRadiiSquared;
    return normalize(normal);
}

vec3 convertGeographicToGeodeticSurfaceNormal(vec3 geographic) {
    float longitude = geographic.x;
    float latitude = geographic.y;

    float cosLatitude = cos(latitude);
    vec3 normal = vec3(
        cosLatitude * cos(longitude),
        sin(latitude),
        cosLatitude * sin(longitude));

    return normal;
}

vec3 convertGeodeticToGeocentric(vec3 geodetic) {
    float height = geodetic.z;

    vec3 n = convertGeographicToGeodeticSurfaceNormal(geodetic);
    vec3 k = ellipsoidRadiiSquared * n;
    float gamma = sqrt(k.x * n.x + k.y * n.y + k.z * n.z);
    // Point on the surface determined as determined by the normal.
    vec3 rSurface = k / gamma;

    return rSurface + (n * height);
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
    float longitude = uTileLongitudeOffset + aPos.x * uTileLongitudeWidth;
    float latitude = uTileLatitudeOffset + aPos.y * uTileLatitudeWidth;

    longitude = radians(longitude);
    latitude = radians(latitude);
    vec3 geographicCoordinates = vec3(longitude, latitude, 0.0);

    vec3 geocentricCoordinates = convertGeodeticToGeocentric(geographicCoordinates);
    if (isTerrainEnabled) {
        float heightDisplacement = getHeightDisplacement(geocentricCoordinates);
        geocentricCoordinates *= heightDisplacement;
    }
    gl_Position = projection * view * model * vec4(geocentricCoordinates, 1.0);
    //geocentricFragPos = vec3(model * vec4(geocentricCoordinates, 1.0));
    geocentricFragPos = geocentricCoordinates;

    vertexColor = vec3(1.0f, 0.5f, 0.2f);
}