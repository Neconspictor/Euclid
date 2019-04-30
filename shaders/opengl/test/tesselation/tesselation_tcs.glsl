#version 450 core

// define the number of CPs in the output patch
layout (vertices = 3) out;

uniform vec3 gEyeWorldPos;

// attributes of the input CPs
in vec3 position_ndc_tcs_in[];
in vec2 texCoord_ndc_tcs_in[];

// attributes of the output CPs
out vec3 position_ndc_tes_in[];
out vec2 texCoord_ndc_tes_in[];

void main()
{
    // Set the control points of the output patch
    TexCoord_ES_in[gl_InvocationID] = TexCoord_CS_in[gl_InvocationID];
    Normal_ES_in[gl_InvocationID] = Normal_CS_in[gl_InvocationID];
    WorldPos_ES_in[gl_InvocationID] = WorldPos_CS_in[gl_InvocationID];
    
}