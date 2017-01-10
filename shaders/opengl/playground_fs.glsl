#version 330 core

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