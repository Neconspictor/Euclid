#version 430 core
out vec4 color;
  
uniform vec4 objectColor;

void main()
{
    color = objectColor;
}