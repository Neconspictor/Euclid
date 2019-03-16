#version 330

in VS_OUT {
    vec2 texCoord;
} fs_in;

out vec4 fragColor;

layout(binding = 0) uniform sampler2D sourceTexture;
layout(binding = 1) uniform sampler2D bloomHalfth;
layout(binding = 2) uniform sampler2D bloomQuarter;
layout(binding = 3) uniform sampler2D bloomEigth;
layout(binding = 4) uniform sampler2D bloomSixteenth;




void main() {

    vec3 color = texture(sourceTexture, fs_in.texCoord).rgb;
    
    // Bloom
    const float strength = 0.5 * 1.0;
    vec3 bloomHalfthSample = texture(bloomHalfth, fs_in.texCoord).rgb * strength;
    vec3 bloomQuarterSample = texture(bloomQuarter, fs_in.texCoord).rgb * strength * 0.75;
    vec3 bloomEigthSample = texture(bloomEigth, fs_in.texCoord).rgb * strength * 0.5;
    vec3 bloomSixteenthSample = texture(bloomSixteenth, fs_in.texCoord).rgb * strength * 0.25;
    color += bloomHalfthSample + bloomQuarterSample + bloomEigthSample + bloomSixteenthSample;

    // HDR tonemapping
    const float exposure = 1.0;
    color *= exposure;
    color = color / (color + vec3(1.0));
	
    // gamma correct
    const float gamma = 2.2f;
    color = pow(color, vec3(1.0/gamma)); 

    fragColor = vec4(color, 1.0);
}