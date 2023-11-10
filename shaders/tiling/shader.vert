#version 400 core
// Offset of the vertex within the tile in longitude and latitude
// in the [0, 1] range.
layout (location = 0) in vec3 aPos;

out vec3 geocentricFragPos;

uniform vec3 ellipsoidRadiiSquared;

uniform float uTileLongitudeOffset;
uniform float uTileLatitudeOffset;
uniform float uTileLongitudeWidth;
uniform float uTileLatitudeWidth;

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


void main()
{
    float longitude = uTileLongitudeOffset + aPos.x * uTileLongitudeWidth;
    float latitude = uTileLatitudeOffset + aPos.y * uTileLatitudeWidth;

    longitude = radians(longitude);
    latitude = radians(latitude);
    vec3 geographicCoordinates = vec3(longitude, latitude, 0.0);

    vec3 geocentricCoordinates = convertGeodeticToGeocentric(geographicCoordinates);

    gl_Position = vec4(geocentricCoordinates, 1.0);
    geocentricFragPos = geocentricCoordinates;
}