#version 330 core
in vec3 vertexColor;
in vec3 FragPos;
in vec3 Normal;

out vec4 FragColor;

uniform vec3 objectColor;
uniform vec3 lightColor;
uniform vec3 lightPos;

uniform sampler2D basicTexture;

float computeLightIntensity(vec3 normal)
{
    // Ambient light
    float ambientStrength = 0.1;

    // Diffuse light
    vec3 lightDir = normalize(lightPos - FragPos);
    float diffuseStrength = max(dot(normal, lightDir), 0.0);

    float intensity = ambientStrength + diffuseStrength;
    return intensity;
}

vec2 computeTextureCoordinates(vec3 normal)
{
    float PI = 3.14159265358979323846;
    float og_oneOverTwoPi = 1.0 / (2.0 * PI);
    float og_oneOverPi = 1.0 / PI;

    return vec2(
        atan(normal.x, normal.z) * og_oneOverTwoPi + 0.5,
        asin(normal.y) * og_oneOverPi + 0.5
    );
}

void main()
{
    vec3 norm = normalize(Normal);
    float lightIntensity = computeLightIntensity(norm);
    vec2 textureCoordinate = computeTextureCoordinates(norm);
    FragColor = lightIntensity * texture(basicTexture, textureCoordinate);
}