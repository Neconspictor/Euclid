#version 450 core
out vec4 colorFinal;

in GS_OUT {
    vec3 normal;
    vec3 tangent;
    vec3 bitangent;
    vec3 color;
} fs_in;

uniform vec4 color;

void main()
{
    colorFinal = vec4(fs_in.color, 1.0);
}  