#version 330 core
in vec3 texCoordsFS;
out vec4 color;

uniform samplerCube skybox;

void main()
{    
    color = texture(skybox, texCoordsFS);
}