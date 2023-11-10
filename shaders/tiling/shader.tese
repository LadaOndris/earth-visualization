#version 400 core

layout(triangles) in;

in vec3 VertexPos[];

out vec3 geocentricFragPos;

void main()
{
    float u = gl_TessCoord.x;
    float v = gl_TessCoord.y;
    float w = gl_TessCoord.z;

    vec3 position = VertexPos[0] * u + VertexPos[1] * v + VertexPos[2] * w;

    geocentricFragPos = position;

    gl_Position = gl_in[0].gl_Position * u + gl_in[1].gl_Position * v + gl_in[2].gl_Position * w;
}