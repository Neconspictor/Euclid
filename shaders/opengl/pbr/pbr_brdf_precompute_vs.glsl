#version 330 core
layout (location = 0) in vec3 position;
layout (location = 1) in vec2 texCoords;

uniform mat4 transform;

out VS_OUT {
	vec2 texCoords;
} vs_out;

void main()
{
	// 2D projection
	//gl_Position = transform * vec4(position.xy, 0.0f, 1.0f);
	gl_Position = vec4(position, 1.0);
	vs_out.texCoords = texCoords;
}