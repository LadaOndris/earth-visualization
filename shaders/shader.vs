#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;

out vec3 vertexColor;
out vec3 FragPos;
out vec3 Normal;


uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    gl_Position = projection * view * model * vec4(aPos, 1.0);
    FragPos = vec3(model * vec4(aPos, 1.0));
    // TODO: compute normal matrix on a CPU for performance reasons
    Normal = mat3(transpose(inverse(model))) *  aNormal;

    vertexColor = vec3(1.0f, 0.5f, 0.2f);
}