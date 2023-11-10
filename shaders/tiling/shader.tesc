#version 400 core

layout (vertices=3) out;

in vec3 geocentricFragPos[];

out vec3 VertexPos[];

float tesselationFactorOuter = 5.0;
float tesselationFactorInner = 5.0;

void main()
{
    gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;
    VertexPos[gl_InvocationID] = geocentricFragPos[gl_InvocationID];

    if (gl_InvocationID == 0) {
        gl_TessLevelOuter[0] = tesselationFactorOuter;
        gl_TessLevelOuter[1] = tesselationFactorOuter;
        gl_TessLevelOuter[2] = tesselationFactorOuter;

        gl_TessLevelInner[0] = tesselationFactorInner;
        gl_TessLevelInner[1] = tesselationFactorInner;
    }
}