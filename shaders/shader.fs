#version 330 core
in vec3 vertexColor;
in vec3 FragPos;

out vec4 FragColor;

uniform vec3 objectColor;
uniform vec3 lightColor;
uniform vec3 lightPos;

uniform sampler2D basicTexture;
// Definition of the ellipsoid
uniform vec3 oneOverRadiiSquared;

vec3 geodeticSurfaceNormalFromWGS84(vec3 point)
{
    vec3 normal = point * oneOverRadiiSquared;
    return normalize(normal);
}

float computeLightIntensity(vec3 normal, vec3 position)
{
    // Ambient light
    float ambientStrength = 0.1;

    // Diffuse light
    vec3 lightDir = normalize(lightPos - position);
    float diffuseStrength = max(dot(normal, lightDir), 0.0);

    float intensity = ambientStrength + diffuseStrength;
    return intensity;
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

    float lightIntensity = computeLightIntensity(normal, FragPos);
    vec2 textureCoordinate = computeTextureCoordinates(normal);
    FragColor = lightIntensity * texture(basicTexture, textureCoordinate);
}