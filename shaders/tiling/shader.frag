#version 330 core
in vec3 vertexColor;
in vec3 geocentricFragPos;

out vec4 FragColor;

// Lighting
uniform vec3 lightPos;

// Textures
uniform sampler2D dayTextureSampler;
uniform vec2 textureGeodeticOffset; // In radians
uniform vec2 textureGridSize;

// Ellipsoid definition
uniform vec3 ellipsoidRadiiSquared;
uniform vec3 ellipsoidOneOverRadiiSquared;

// Grid definition
uniform float gridResolution;
uniform float gridLineWidth;

// Enable/disable features
uniform bool useDayTexture;
uniform bool useNightTexture;
uniform bool useHeightMapTexture;
uniform bool displayGrid;

const float PI = 3.14159265358979323846;
const float oneOverTwoPi = 1.0 / (2.0 * PI);
const float oneOverPi = 1.0 / PI;
const float eps = 0.001;


vec3 convertGeocentricToGeocentricSurfaceNormal(vec3 point)
{
    vec3 normal = point * ellipsoidOneOverRadiiSquared;
    return normalize(normal);
}

float computeDiffuseLight(vec3 normal, vec3 position)
{
    vec3 lightDir = normalize(lightPos - position);
    float diffuse = dot(normal, lightDir);
    return diffuse;
}

float computeLightIntensity(float diffuseIntensity)
{
    // Ambient light
    float ambientStrength = 0.1;
    float diffuseStrength = max(diffuseIntensity, 0.0);
    float intensity = ambientStrength + diffuseStrength;
    return intensity;
}

vec4 computeDayColor(vec3 normal, vec2 textureCoordinates, float diffuseStrength)
{
    float lightIntensity = computeLightIntensity(diffuseStrength);
    return lightIntensity * texture(dayTextureSampler, textureCoordinates);
}

vec2 computeTextureCoordinates(vec3 normal)
{
    return vec2(
        atan(normal.z, normal.x) * oneOverTwoPi + 0.5,
        asin(normal.y) * oneOverPi + 0.5
    );
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

bool isGrid(vec2 globalTextureCoordinates) {
    vec2 distanceToLine = mod(globalTextureCoordinates, gridResolution);
    vec2 dx = abs(dFdx(globalTextureCoordinates));
    vec2 dy = abs(dFdy(globalTextureCoordinates));
    vec2 dF = vec2(max(dx.s, dy.s),
                   max(dx.t, dy.t)) * gridLineWidth;
    return any(lessThan(distanceToLine, dF));
}

void main()
{

    if (!useDayTexture) {
        FragColor = vec4(0.2, 0.6, 0.2, 1);
        return;
    }

    vec3 normal = convertGeocentricToGeocentricSurfaceNormal(geocentricFragPos);
    float diffuseIntensity = computeDiffuseLight(normal, geocentricFragPos);
    vec2 globalTextureCoordinates = computeTextureCoordinates(normal);

    // Use the following direct approach to handle the problem with the poles
    float normalizedLongitude = (textureGeodeticOffset[0] + PI) / (2.0 * PI);
    float normalizedLatitude = (textureGeodeticOffset[1] + PI / 2.0) / PI;
    vec2 textureCoordinatesOffset = vec2(normalizedLongitude, normalizedLatitude);

    vec2 tileTextureCoordinates = (globalTextureCoordinates - textureCoordinatesOffset) * textureGridSize;

    // Global coords (of the tile) should always be larger than the
    // offset of the texture for the tile.
    if (textureCoordinatesOffset[0] > globalTextureCoordinates[0]) {
        FragColor = vec4(1, 0, 0, 1);
        return;
    }
    if (textureCoordinatesOffset[1] > globalTextureCoordinates[1]) {
        FragColor = vec4(0, 1, 0, 1);
        return;
    }

    if (displayGrid && isGrid(globalTextureCoordinates)) {
        FragColor = vec4(0.3, 0.3, 0.3, 1);
    }
    else {
        FragColor = texture(dayTextureSampler, tileTextureCoordinates);
    }
}