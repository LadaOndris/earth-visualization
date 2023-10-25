#version 330 core
in vec3 vertexColor;
in vec3 FragPos;

out vec4 FragColor;

void main()
{
    FragColor = vec4(0.1, 0.7, 0.3, 1);
}