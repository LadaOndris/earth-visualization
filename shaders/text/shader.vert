#version 330 core

layout (location = 0) in vec4 vertex; // <vec2 pos, vec2 tex>
layout (location = 1) in vec2 instancePosition;

out vec2 TexCoords;

uniform mat4 view;
uniform mat4 projection;

uniform vec3 ellipsoidRadiiSquared;

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
    float longitude = radians(instancePosition[1]);
    float latitude = radians(instancePosition[0]);
    vec3 geographicCoordinates = vec3(longitude, latitude, 0.0);
    vec3 geocentricCoordinates = convertGeodeticToGeocentric(geographicCoordinates);

    vec4 projectedPosition = projection * view * vec4(geocentricCoordinates, 1.0);
    projectedPosition /= projectedPosition.w;
    gl_Position = vec4(projectedPosition.xy + vertex.xy / 1000, projectedPosition.z, 1.0);

    TexCoords = vertex.zw;
}