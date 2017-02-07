#version 330 core
#extension GL_ARB_shading_language_include : require

#define SMAA_RT_METRICS float4(1.0 / 1280.0, 1.0 / 720.0, 1280.0, 720.0)
#define SMAA_GLSL_3
#define SMAA_PRESET_HIGH
#include "SMAA/SMAA.hlsl"


in GS_OUT {
    vec2 tex;
} fs_in;

// Ouput data
out vec4 color;

uniform float mixValue;

uniform sampler2D diffuse;
uniform sampler2D diffuseMultiply;


void main() {
	color = mix(texture(diffuse, fs_in.tex), texture(diffuseMultiply, fs_in.tex), mixValue);
}