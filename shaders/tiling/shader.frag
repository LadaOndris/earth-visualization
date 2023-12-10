#version 400 core


in TE_OUT {
    vec3 geocentricFragPos;
    vec3 surfaceNormal;
} fs_in;

out vec4 FragColor;

// Lighting
uniform vec3 lightPos;

// Textures
uniform sampler2D dayTextureSampler;
uniform vec2 dayTextureGeodeticOffset; // In radians
uniform vec2 dayTextureGridSize;

uniform sampler2D nightTextureSampler;
uniform vec2 nightTextureGeodeticOffset; // In radians
uniform vec2 nightTextureGridSize;

// Height map
uniform sampler2D heightMapSampler;
uniform vec2 heightMapGeodeticOffset; // In radians
uniform vec2 heightMapGridSize;
uniform float heightDisplacementFactor;
uniform int heightScale;

uniform float blendDuration;
uniform float blendDurationScale;

// Ellipsoid definition
uniform vec3 ellipsoidRadiiSquared;
uniform vec3 ellipsoidOneOverRadiiSquared;

// Grid definition
uniform float gridResolution;
uniform float gridLineWidth;

// Enable/disable features
uniform bool useDayTexture;
uniform bool isNightEnabled;
uniform bool useHeightMapTexture;
uniform bool displayGrid;
uniform bool isTerrainShadingEnabled;

const float PI = 3.14159265358979323846;
const float oneOverTwoPi = 1.0 / (2.0 * PI);
const float oneOverPi = 1.0 / PI;
const float eps = 0.001;

vec3 convertGeocentricToGeocentricSurfaceNormal(vec3 point) {
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

vec2 calcTileDayTextureCoordinates(vec2 globalTextureCoordinates) {
    // Use the following direct approach to handle the problem with the poles
    float normalizedLongitude = (dayTextureGeodeticOffset[0] + PI) / (2.0 * PI);
    float normalizedLatitude = (dayTextureGeodeticOffset[1] + PI / 2.0) / PI;
    vec2 textureCoordinatesOffset = vec2(normalizedLongitude, normalizedLatitude);

    vec2 tileTextureCoordinates = (globalTextureCoordinates - textureCoordinatesOffset) * dayTextureGridSize;
    return tileTextureCoordinates;
}

vec2 calcTileNightTextureCoordinates(vec2 globalTextureCoordinates) {
    // Use the following direct approach to handle the problem with the poles
    float normalizedLongitude = (nightTextureGeodeticOffset[0] + PI) / (2.0 * PI);
    float normalizedLatitude = (nightTextureGeodeticOffset[1] + PI / 2.0) / PI;
    vec2 textureCoordinatesOffset = vec2(normalizedLongitude, normalizedLatitude);

    vec2 tileTextureCoordinates = (globalTextureCoordinates - textureCoordinatesOffset) * nightTextureGridSize;
    return tileTextureCoordinates;
}

vec4 computeDayColor(vec2 globalTextureCoordinates, float diffuseStrength)
{
    vec2 tileTextureCoordinates = calcTileDayTextureCoordinates(globalTextureCoordinates);
    float lightIntensity = computeLightIntensity(diffuseStrength);
    return lightIntensity * texture(dayTextureSampler, tileTextureCoordinates);
}

vec4 computeNightColor(vec2 globalTextureCoordinates)
{
    vec2 tileTextureCoordinates = calcTileNightTextureCoordinates(globalTextureCoordinates);
    return texture(nightTextureSampler, tileTextureCoordinates);
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

vec4 blendDayAndNight(vec2 globalTextureCoordinates, float diffuseIntensity) {
    if (diffuseIntensity > blendDuration)
    {
        return computeDayColor(globalTextureCoordinates, diffuseIntensity);
    }
    else if (diffuseIntensity < -blendDuration)
    {
        return computeNightColor(globalTextureCoordinates);
    }
    else
    {
        vec4 dayColor = computeDayColor(globalTextureCoordinates, diffuseIntensity);
        vec4 nightColor = computeNightColor(globalTextureCoordinates);
        float ratio = (diffuseIntensity + blendDuration) * blendDurationScale;
        return mix(nightColor, dayColor, ratio);
    }
}

mat3 construct_tbn_matrix(vec3 surfaceNormal, vec3 worldPosition, vec2 textureCoords)
{
    vec3 Q1 = dFdx(worldPosition);
    vec3 Q2 = dFdy(worldPosition);
    vec2 st1 = dFdx(textureCoords);
    vec2 st2 = dFdy(textureCoords);

    vec3 T = normalize(Q1 * st2.t - Q2 * st1.t);

    // Reorthogonalize
    T = normalize(T - dot(T, surfaceNormal) * surfaceNormal);
    vec3 B = cross(surfaceNormal, T);

    mat3 tbn = mat3(T, B, surfaceNormal);
    return tbn;
}

vec2 calcTileHeightTextureCoordinates(vec2 globalTextureCoordinates) {
    // Use the following direct approach to handle the problem with the poles
    float normalizedLongitude = (heightMapGeodeticOffset[0] + PI) / (2.0 * PI);
    float normalizedLatitude = (heightMapGeodeticOffset[1] + PI / 2.0) / PI;
    vec2 textureCoordinatesOffset = vec2(normalizedLongitude, normalizedLatitude);

    vec2 tileTextureCoordinates = (globalTextureCoordinates - textureCoordinatesOffset) * heightMapGridSize;
    return tileTextureCoordinates;
}


vec2 computeNormalSobelFilter(vec2 position, sampler2D heightMap) {
    float coeff = 1.0 / 255.0;
    vec2 tileTextureSize = textureSize(heightMap, 0);
    float upperLeft = texture(heightMap, position.xy + vec2(-1.0, 1.0) / tileTextureSize).r * coeff;
    float upperCenter = texture(heightMap, position.xy + vec2(0.0, 1.0) / tileTextureSize).r * coeff;
    float upperRight = texture(heightMap, position.xy + vec2(1.0, 1.0) / tileTextureSize).r * coeff;
    float left = texture(heightMap, position.xy + vec2(-1.0, 0.0) / tileTextureSize).r * coeff;
    float right = texture(heightMap, position.xy + vec2(1.0, 0.0) / tileTextureSize).r * coeff;
    float lowerLeft = texture(heightMap, position.xy + vec2(-1.0, -1.0) / tileTextureSize).r * coeff;
    float lowerCenter = texture(heightMap, position.xy + vec2(0.0, -1.0) / tileTextureSize).r * coeff;
    float lowerRight = texture(heightMap, position.xy + vec2(1.0, -1.0) / tileTextureSize).r * coeff;

    float x = upperRight + (2.0 * right) + lowerRight -
    upperLeft - (2.0 * left) - lowerLeft;
    float y = lowerLeft + (2.0 * lowerCenter) + lowerRight -
    upperLeft - (2.0 * upperCenter) - upperRight;

    return vec2(x, y);
}

void main()
{
    if (!useDayTexture) {
        FragColor = vec4(0.2, 0.6, 0.2, 1);
        return;
    }

    vec2 globalTextureCoordinates = computeTextureCoordinates(fs_in.surfaceNormal);

    if (displayGrid && isGrid(globalTextureCoordinates)) {
        FragColor = vec4(0.3, 0.3, 0.3, 1);
    }
    else {
        if (isNightEnabled) {
            vec3 normal = fs_in.surfaceNormal;
            if (isTerrainShadingEnabled) {
                mat3 tbn = construct_tbn_matrix(normal, fs_in.geocentricFragPos, globalTextureCoordinates);
                vec2 heightMapTextureCoords = calcTileHeightTextureCoordinates(globalTextureCoordinates);
                vec2 gradient = computeNormalSobelFilter(heightMapTextureCoords, heightMapSampler);
                vec3 geometryNormal = normalize(vec3(gradient * heightScale, 1.0));
                vec3 normal = tbn * geometryNormal;

                float diffuseIntensity = computeDiffuseLight(normal, fs_in.geocentricFragPos);
                FragColor = blendDayAndNight(globalTextureCoordinates, diffuseIntensity);
            }
            else {
                float diffuseIntensity = computeDiffuseLight(normal, fs_in.geocentricFragPos);
                FragColor = blendDayAndNight(globalTextureCoordinates, diffuseIntensity);
            }
        }
        else {
            FragColor = computeDayColor(globalTextureCoordinates, 1.0);
        }
    }
}