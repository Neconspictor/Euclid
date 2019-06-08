#version 430 core

in vec3 interpolatedVertexColor;
out vec4 color;

void main()
{
    color = vec4(interpolatedVertexColor, 1.0);
}