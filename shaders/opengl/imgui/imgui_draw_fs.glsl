#version 420

in vec2 Frag_UV;
in vec4 Frag_Color;
out vec4 Out_Color;

layout(binding = 0) uniform sampler2D Texture;

void main()
{
    Out_Color = Frag_Color * texture( Texture, Frag_UV.st);
}