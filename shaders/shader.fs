#version 330 core
in vec3 vertexColor;
in vec3 FragPos;

out vec4 FragColor;

uniform vec3 objectColor;
uniform vec3 lightColor;
uniform vec3 lightPos;

uniform sampler2D dayTexture;
uniform sampler2D nightTexture;
uniform float blendDuration;
uniform float blendDurationScale;
// Definition of the ellipsoid
uniform vec3 oneOverRadiiSquared;

vec3 geodeticSurfaceNormalFromWGS84(vec3 point)
{
    vec3 normal = point * oneOverRadiiSquared;
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
    return lightIntensity * texture(dayTexture, textureCoordinates);
}

vec4 computeNightColor(vec2 textureCoordinates)
{
    return texture(nightTexture, textureCoordinates);
}

vec2 computeTextureCoordinates(vec3 normal)
{
    float PI = 3.14159265358979323846;
    float oneOverTwoPi = 1.0 / (2.0 * PI);
    float oneOverPi = 1.0 / PI;

    return vec2(
        atan(normal.x, normal.z) * oneOverTwoPi + 0.5,
        asin(normal.y) * oneOverPi + 0.5
    );
}


void main()
{
    // Compute on GPU per fragment to avoid interpolation of normals if computed per vertex
    vec3 normal = geodeticSurfaceNormalFromWGS84(FragPos);
    float diffuseIntensity = computeDiffuseLight(normal, FragPos);
    vec2 textureCoordinates = computeTextureCoordinates(normal);

    if (diffuseIntensity > blendDuration)
    {
        FragColor = computeDayColor(normal, textureCoordinates, diffuseIntensity);
    }
    else if (diffuseIntensity < -blendDuration)
    {
        FragColor = computeNightColor(textureCoordinates);
    }
    else
    {
        vec4 dayColor = computeDayColor(normal, textureCoordinates, diffuseIntensity);
        vec4 nightColor = computeNightColor(textureCoordinates);
        float ratio = (diffuseIntensity + blendDuration) * blendDurationScale;
        FragColor = mix(nightColor, dayColor, ratio);
    }
}