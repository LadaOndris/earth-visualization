#version 330 core

layout (location = 0) in vec4 vertex; // <vec2 pos, vec2 tex>
layout (location = 1) in vec3 instancePosition;

out vec2 TexCoords;

uniform mat4 view;
uniform mat4 projection;

void main()
{
    vec4 projectedPosition = projection * view * vec4(instancePosition, 1.0);
    projectedPosition /= projectedPosition.w;
    gl_Position = vec4(projectedPosition.xy + vertex.xy / 1000, projectedPosition.z, 1.0);

    TexCoords = vertex.zw;
}