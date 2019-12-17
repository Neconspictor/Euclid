#version 460 core

// define the number of CPs in the output patch
layout (vertices = 4) out;

// attributes of the input CPs
in VS_OUT {
    vec3 normal;
    vec3 tangent;
    vec3 bitangent;
    vec3 positionViewSpace;
} tcs_in[];

// attributes of the output CPs
out TCS_OUT{
    vec3 normal;
    vec3 tangent;
    vec3 bitangent;
    vec3 positionViewSpace;
} tes_in[];


uniform uint outerLevel0;
uniform uint outerLevel1;
uniform uint outerLevel2;
uniform uint outerLevel3;
uniform uint innerLevel0;
uniform uint innerLevel1;


void main()
{
    gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;
    tes_in[gl_InvocationID].normal = tcs_in[gl_InvocationID].normal;
    tes_in[gl_InvocationID].tangent = tcs_in[gl_InvocationID].tangent;
    tes_in[gl_InvocationID].bitangent = tcs_in[gl_InvocationID].bitangent;
    tes_in[gl_InvocationID].positionViewSpace = tcs_in[gl_InvocationID].positionViewSpace;
    
    gl_TessLevelOuter[0] = outerLevel0;
    gl_TessLevelOuter[1] = outerLevel1;
    gl_TessLevelOuter[2] = outerLevel2;
    gl_TessLevelOuter[3] = outerLevel3;
    gl_TessLevelInner[0] = innerLevel0;
    gl_TessLevelInner[1] = innerLevel1;
}