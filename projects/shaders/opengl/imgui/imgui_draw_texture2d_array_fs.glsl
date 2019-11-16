#version 420

in vec2 Frag_UV;
in vec4 Frag_Color;
out vec4 Out_Color;

layout(binding = 0) uniform sampler2DArray Texture;
uniform float Index;
uniform int MipMapLevel;
uniform int UseTransparency;
uniform int UseGammaCorrection;
uniform int UseToneMapping;
uniform mat3 transformUV;
uniform int FlipY;

void main()
{
    vec4 color = textureLod( Texture, vec3(Frag_UV, Index), MipMapLevel);
    
    float depth = textureLod( Texture, vec3(Frag_UV, Index), MipMapLevel).r;    
    
    if (bool(UseToneMapping)) {
        const float exposure = 1.0;
        color *= exposure;
        color.rgb = color.rgb / (color.rgb + vec3(1.0));
    }
    
    if (bool(UseGammaCorrection)) {
        // gamma correct
        const float gamma = 2.2f;
        color.rgb = pow(color.rgb, vec3(1.0/gamma)); 
    }

    if (bool(UseTransparency)) {
        Out_Color = Frag_Color * color;
    } else {
        Out_Color = Frag_Color * vec4(color.rgb, 1.0);
    }
}