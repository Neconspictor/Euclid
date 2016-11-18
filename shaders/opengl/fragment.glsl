#version 330 core

// input data
in vec4 colorFragment;
in vec2 texCoordFrag;

// Ouput data
out vec4 color;

uniform float mixValue;

uniform sampler2D diffuse;
uniform sampler2D diffuseMultiply;


void main() {
	color = mix(texture(diffuse, texCoordFrag), texture(diffuseMultiply, texCoordFrag), mixValue);
}