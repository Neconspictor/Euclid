#version 410 compatibility
#extension GL_ARB_shading_language_include : require

#include "test.glsl"

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
    
    foo();
}