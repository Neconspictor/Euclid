#version 460 core

// define the number of CPs in the output patch
layout (vertices = 3) out;

// attributes of the input CPs
in vec4 position_ndc_tcs_in[];
in vec2 texCoord_ndc_tcs_in[];

// attributes of the output CPs
out vec4 position_ndc_tes_in[];
out vec2 texCoord_ndc_tes_in[];


uniform uint outerLevel0;
uniform uint outerLevel1;
uniform uint outerLevel2;
uniform uint innerLevel0;


void main()
{
    position_ndc_tes_in[gl_InvocationID] = position_ndc_tcs_in[gl_InvocationID];
    texCoord_ndc_tes_in[gl_InvocationID] = texCoord_ndc_tcs_in[gl_InvocationID];
    
    gl_TessLevelOuter[0] = outerLevel0;
    gl_TessLevelOuter[1] = outerLevel1;
    gl_TessLevelOuter[2] = outerLevel2;
    gl_TessLevelInner[0] = innerLevel0;
}