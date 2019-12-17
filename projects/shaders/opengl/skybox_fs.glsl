#version 460 core

in vec3 texCoordsFS;
out vec4 color;

uniform samplerCube skybox;

void main()
{    
    color = textureLod(skybox, normalize(texCoordsFS), 0);
}