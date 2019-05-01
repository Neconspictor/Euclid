#version 450 core

// define the number of CPs in the output patch
layout (vertices = 4) out;

// attributes of the input CPs
in vec2 texCoord_ndc_tcs_in[];

// attributes of the output CPs
out vec2 texCoord_ndc_tes_in[];


uniform uint outerLevel0;
uniform uint outerLevel1;
uniform uint outerLevel2;
uniform uint outerLevel3;
uniform uint innerLevel0;
uniform uint innerLevel1;


void main()
{
    gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;
    texCoord_ndc_tes_in[gl_InvocationID] = texCoord_ndc_tcs_in[gl_InvocationID];
    
    gl_TessLevelOuter[0] = outerLevel0;
    gl_TessLevelOuter[1] = outerLevel1;
    gl_TessLevelOuter[2] = outerLevel2;
    gl_TessLevelOuter[3] = outerLevel3;
    gl_TessLevelInner[0] = innerLevel0;
    gl_TessLevelInner[1] = innerLevel1;
}