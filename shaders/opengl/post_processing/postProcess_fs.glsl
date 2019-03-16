#version 330

in VS_OUT {
    vec2 texCoord;
} fs_in;

out vec4 fragColor;

layout(binding = 0) uniform sampler2D sourceTexture;
layout(binding = 1) uniform sampler2D glowTexture;




void main() {

    vec3 color = texture(sourceTexture, fs_in.texCoord).rgb;
    
    // Bloom
    const float strength = 1.0;
    vec3 bloom = texture(glowTexture, fs_in.texCoord).rgb * strength;
    color += bloom;

    // HDR tonemapping
    const float exposure = 1.0;
    color *= exposure;
    color = color / (color + vec3(1.0));
	
    // gamma correct
    const float gamma = 2.2f;
    color = pow(color, vec3(1.0/gamma)); 

    fragColor = vec4(color, 1.0);
}