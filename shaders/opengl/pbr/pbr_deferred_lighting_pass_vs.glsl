#version 400 core
layout (location = 0) in vec4 position;
layout (location = 1) in vec2 texCoords;


out VS_OUT {	
	vec2 tex_coords;
} vs_out;

void main()
{
    gl_Position = position;
    vs_out.tex_coords= texCoords;
}