#version 410 compatibility
//#extension GL_ARB_shading_language_include : require

//#define SMAA_RT_METRICS float4(1.0 / 1280.0, 1.0 / 720.0, 1280.0, 720.0)
//#define SMAA_GLSL_3
//#define SMAA_PRESET_HIGH

// Undefine fragment shader specific aspects as the glsls compiler has problems 
// with the discard keyword while compiling vertex shaders
//#define SMAA_INCLUDE_PS 0
//#include "SMAA/SMAA.hlsl"

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 texCoord;

uniform mat4 transform;

out VS_OUT {
    vec2 tex;
    
} vs_out;

void main() {
	gl_Position = transform * vec4(position, 1.0f);
	vs_out.tex = texCoord;
    
    //foo();
}