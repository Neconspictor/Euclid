#version 330 core
layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 texCoord;

uniform mat4 transform;

out vec2 texCoordFrag;

void main() {
	gl_Position = transform * vec4(position, 1.0f);
	texCoordFrag = texCoord;
}