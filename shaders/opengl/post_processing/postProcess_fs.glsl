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
layout(binding = 5) uniform sampler2D aoMap;




void main() {

    vec4 color = texture(sourceTexture, fs_in.texCoord).rgba;
    
    // Bloom
    const float strength = 0.5 * 1.0;
    vec4 bloomHalfthSample = texture(bloomHalfth, fs_in.texCoord) * strength;
    vec4 bloomQuarterSample = texture(bloomQuarter, fs_in.texCoord) * strength * 0.75;
    vec4 bloomEigthSample = texture(bloomEigth, fs_in.texCoord) * strength * 0.5;
    vec4 bloomSixteenthSample = texture(bloomSixteenth, fs_in.texCoord) * strength * 0.25;
    color += bloomHalfthSample + bloomQuarterSample + bloomEigthSample + bloomSixteenthSample;

    // HDR tonemapping
    const float exposure = 1.0;
    color *= exposure;
    color = color / (color + vec4(1.0));
	
    // gamma correct
    const float gamma = 2.2f;
    color = pow(color, vec4(1.0/gamma)); 
    
    // Ambient Occlusion
    color.rgb *= texture(aoMap, fs_in.texCoord).r;

    fragColor = color;
}