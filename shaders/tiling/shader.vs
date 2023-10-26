#version 330 core
// Offset of the vertex within the tile in longitude and latitude
layout (location = 0) in vec3 aPos;
// layout (location = 1) in vec3 aNormal;

out vec3 vertexColor;
out vec3 FragPos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

uniform vec3 ellipsoidRadiiSquared;
uniform vec3 ellipsoidOneOverRadiiSquared;

uniform float uTileLongitudeOffset;
uniform float uTileLatitudeOffset;
uniform float uTileLongitudeWidth;
uniform float uTileLatitudeWidth;

vec3 convertGeocentricToGeocentricSurfaceNormal(vec3 point)  {
    vec3 normal = point * ellipsoidOneOverRadiiSquared;
    return normalize(normal);
}

vec3 convertGeocentricToGeodetic(vec3 point) {
    vec3 normal = convertGeocentricToGeocentricSurfaceNormal(point);
    vec3 geographic = vec3(atan(normal.y, normal.x), asin(normal.z / length(normal)), 0.0);
    return geographic;
}

vec3 convertGeographicToGeodeticSurfaceNormal(vec3 geographic) {
    float longitude = geographic.x;
    float latitude = geographic.y;

    float cosLatitude = cos(latitude);
    vec3 normal = vec3(
        cosLatitude * cos(longitude),
        cosLatitude * sin(longitude),
        sin(latitude));

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

    gl_Position = projection * view * model * vec4(geocentricCoordinates, 1.0);
    FragPos = vec3(model * vec4(geocentricCoordinates, 1.0));

    vertexColor = vec3(1.0f, 0.5f, 0.2f);
}