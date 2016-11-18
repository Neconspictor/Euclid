#version 330 core
layout(location = 0) in vec3 position;
layout(location = 1) in vec3 vertexColor;
layout(location = 2) in vec2 texCoord;

uniform mat4 transform;

out vec4 colorFragment;
out vec2 texCoordFrag;

void main() {
	gl_Position = transform * vec4(position, 1.0f);
	colorFragment = vec4(vertexColor, 1.0f);
	texCoordFrag = texCoord;
}