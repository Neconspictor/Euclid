#version 460 core

#define SMAA_INCLUDE_VS 0
#define SMAA_INCLUDE_PS 1
#include "post_processing/SMAA/SMAA_base.glsl"
#include "post_processing/SMAA/SMAA.hlsl"

in VS_OUT {
	vec2 tex_coords;
    vec2 pixel_coords;
    vec4 offset[3];
} fs_in;

layout(binding = 0) uniform sampler2D edgeTex;
layout(binding = 1) uniform sampler2D areaTex;
layout(binding = 2) uniform sampler2D searchTex;

layout(location = 0) out vec4 FragColor;


void main() {

    // Only needed for temporal reprojection - so we just use 0 for now
    vec4 subsampleIndices = vec4(0.0);
    
    FragColor = SMAABlendingWeightCalculationPS(fs_in.tex_coords, 
        fs_in.pixel_coords, 
        fs_in.offset, 
        edgeTex, 
        areaTex, 
        searchTex, subsampleIndices);
}