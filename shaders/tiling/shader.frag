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

const float PI = 3.14159265358979323846;
float oneOverTwoPi = 1.0 / (2.0 * PI);
float oneOverPi = 1.0 / PI;

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
        atan(normal.x, normal.z) * oneOverTwoPi + 0.5,
        asin(normal.y) * oneOverPi + 0.5
    );
}

vec2 convertCoordinatesToTextureTile(vec2 globalTexCoords)
{
    // globalTexCoords are in range [0, 1]
    // textureGeodeticOffset are in range [0, 2pi] and [0, pi]
    // The offset in global map is being normalized to the [0, 1] range.
    float longitudeOffsetNormalized = textureGeodeticOffset[0] * oneOverTwoPi;
    float latitudeOffsetNormalized = textureGeodeticOffset[1] * oneOverPi;

    vec2 textureGeodeticOffsetNormalized = vec2(
        longitudeOffsetNormalized, latitudeOffsetNormalized
    );
    // TODO: we must not subtract Tex Coords and Geo Coords (different "units")
    // We need to shift the tile to the beginning of the coordinate system
    // and scale it according to the fraction of the tile.
    return (globalTexCoords - textureGeodeticOffsetNormalized) * textureGridSize;
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
    vec3 normal = convertGeocentricToGeocentricSurfaceNormal(geocentricFragPos);
    float diffuseIntensity = computeDiffuseLight(normal, geocentricFragPos);
    vec2 globalTextureCoordinates = computeTextureCoordinates(normal);

    vec3 textureGeocentricOffset = convertGeodeticToGeocentric(vec3(textureGeodeticOffset, 0));
    vec3 textureOffsetNormal = convertGeocentricToGeocentricSurfaceNormal(textureGeocentricOffset);
    vec2 textureCoordinatesOffset = computeTextureCoordinates(textureOffsetNormal);

    vec2 tileTextureCoordinates = (globalTextureCoordinates - textureCoordinatesOffset) * textureGridSize;

    // Global coords should always be larger...
    // wait? But the texture coordiantes are normalized.. to the same scale.
    // The range of texture coordinates is somewhere between [0, 1] depending on
    // which range of the globe it covers.
    //    if (textureGeodeticOffsetNormalized[0] > globalTextureCoordinates[0]) {
    //        FragColor = vec4(1, 0, 0, 1);
    //        return;
    //    }
    //    if (textureGeodeticOffsetNormalized[1] > globalTextureCoordinates[1]) {
    //        FragColor = vec4(0, 1, 0, 1);
    //        return;
    //    }

    //vec2 tileTextureCoordinates = convertCoordinatesToTextureTile(globalTextureCoordinates);

    if (textureCoordinatesOffset[0] < 0 ||
    textureCoordinatesOffset[1] < 0) {
        FragColor = vec4(1, 0, 0, 1);
        return;
    }

    if (textureCoordinatesOffset[0] > 1 ||
    textureCoordinatesOffset[1] > 1) {
        FragColor = vec4(0, 1, 0, 1);
        return;
    }

    FragColor = texture(dayTextureSampler, globalTextureCoordinates); // Should be in [0, 1]
    //FragColor = computeDayColor(normal, tileTextureCoordinates, diffuseIntensity);
    //FragColor = vec4(tileTextureCoordinates[0], 0, 0, 1);
    //FragColor = vec4(normal, 1);
    // FragColor = vec4(0.1, 0.7, 0.3, 1);
}