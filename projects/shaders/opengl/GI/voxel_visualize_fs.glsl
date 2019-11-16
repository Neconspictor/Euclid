#version 430 core

layout(location = 0) out vec4 FragColor;

in GS_OUT {
    vec4 color;
} fs_in;

void main()
{
    FragColor = fs_in.color;
    //FragColor = vec4(1.0);
}  