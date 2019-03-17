#define SMAA_INCLUDE_VS 1
#define SMAA_INCLUDE_PS 0
#include "post_processing/SMAA/SMAA_base.glsl"
#include "post_processing/SMAA/SMAA.hlsl"

layout(location = 0) in vec4 position;
layout(location = 1) in vec2 texcoord;

out VS_OUT {
	vec2 tex_coords;
    vec4 offset[3];
} vs_out;


void main() {
    gl_Position = position;
    vs_out.tex_coords = texcoord;
    SMAAEdgeDetectionVS(texcoord, vs_out.offset);
}