#version 430

#define SMAA_INCLUDE_VS 0
#define SMAA_INCLUDE_PS 1
#include "post_processing/SMAA/SMAA_base.glsl"
#include "post_processing/SMAA/SMAA.hlsl"

in VS_OUT {
	vec2 tex_coords;
    vec4 offset;
} fs_in;

layout(binding = 0) uniform sampler2D blendTex;
layout(binding = 1) uniform sampler2D colorTex;

layout(location = 0) out vec4 FragColor;


void main() {
    
    FragColor = SMAANeighborhoodBlendingPS(fs_in.tex_coords, 
        fs_in.offset, 
        colorTex,
        blendTex);
}