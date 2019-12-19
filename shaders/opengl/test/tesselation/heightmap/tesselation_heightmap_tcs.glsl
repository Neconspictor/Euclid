#version 460 core

// define the number of CPs in the output patch
layout (vertices = 4) out;

in VS_OUT {
    vec3 normal;
    vec2 texCoords;
} vs_out[];

out TCS_OUT {
    vec3 normal;
    vec2 texCoords;
} tcs_out[];



uniform uint outerLevel0;
uniform uint outerLevel1;
uniform uint outerLevel2;
uniform uint outerLevel3;
uniform uint innerLevel0;
uniform uint innerLevel1;


void main()
{
    gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;
    tcs_out[gl_InvocationID].normal = vs_out[gl_InvocationID].normal;
    tcs_out[gl_InvocationID].texCoords = vs_out[gl_InvocationID].texCoords;
    
    gl_TessLevelOuter[0] = outerLevel0;
    gl_TessLevelOuter[1] = outerLevel1;
    gl_TessLevelOuter[2] = outerLevel2;
    gl_TessLevelOuter[3] = outerLevel3;
    gl_TessLevelInner[0] = innerLevel0;
    gl_TessLevelInner[1] = innerLevel1;
}