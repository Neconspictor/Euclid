#version 460 core

layout(location = 0) out vec4 color;
layout(location = 1) out vec4 luminance;
  
uniform vec4 objectColor;

void main()
{
    color = objectColor;
	luminance = vec4(0.0);
}