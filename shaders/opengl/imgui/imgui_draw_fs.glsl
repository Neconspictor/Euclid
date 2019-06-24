#version 420

in vec2 Frag_UV;
in vec4 Frag_Color;
out vec4 Out_Color;

layout(binding = 0) uniform sampler2D Texture;
uniform int MipMapLevel;
uniform int UseTransparency;
uniform int UseGammaCorrection;
uniform int UseToneMapping;

void main()
{
    vec4 color = textureLod( Texture, Frag_UV.st, MipMapLevel);
    
    if (UseToneMapping) {
        const float exposure = 1.0;
        color *= exposure;
        color.rgb = color.rgb / (color.rgb + vec3(1.0));
    }
    
    if (UseGammaCorrection) {
        // gamma correct
        const float gamma = 2.2f;
        color.rgb = pow(color.rgb, vec3(1.0/gamma)); 
    }

    if (UseTransparency) {
        Out_Color = Frag_Color * color;
    } else {
        Out_Color = Frag_Color * vec4(color.rgb, 1.0);
    }
    
    
}