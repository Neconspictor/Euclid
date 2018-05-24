#version 400 core
layout (location = 0) in vec3 position;
layout (location = 1) in vec2 texCoords;

uniform mat4 transform;


out VS_OUT {	
	vec2 tex_coords;
} vs_out;

void main()
{
    gl_Position = transform * vec4(position.xy, 0.0f, 1.0f);
    vs_out.tex_coords= texCoords;
}