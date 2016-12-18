#version 330 core
out vec4 color;
  
uniform vec4 objectColor;
uniform vec4 lightColor;

void main()
{
    color = lightColor * objectColor;
}