#version 400 core
// Offset of the vertex within the tile in longitude and latitude
// in the [0, 1] range.
layout (location = 0) in vec3 aPos;

out VS_OUT {
    vec3 geocentricFragPos;
    vec3 surfaceNormal;
} vs_out;

uniform vec3 ellipsoidRadiiSquared;

uniform float uTileLongitudeOffset;
uniform float uTileLatitudeOffset;
uniform float uTileLongitudeWidth;
uniform float uTileLatitudeWidth;

uniform vec3 ellipsoidOneOverRadiiSquared;

vec3 convertGeocentricToGeocentricSurfaceNormal(vec3 point)
{
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


void main()
{
    float longitude = uTileLongitudeOffset + aPos.x * uTileLongitudeWidth;
    float latitude = uTileLatitudeOffset + aPos.y * uTileLatitudeWidth;

    longitude = radians(longitude);
    latitude = radians(latitude);
    vec3 geographicCoordinates = vec3(longitude, latitude, 0.0);

    vec3 geocentricCoordinates = convertGeodeticToGeocentric(geographicCoordinates);
    vec3 surfaceNormal = convertGeocentricToGeocentricSurfaceNormal(geocentricCoordinates);

    gl_Position = vec4(geocentricCoordinates, 1.0);
    vs_out.geocentricFragPos = geocentricCoordinates;
    vs_out.surfaceNormal = surfaceNormal;
}