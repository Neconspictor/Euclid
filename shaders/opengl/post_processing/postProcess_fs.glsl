#version 420

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
layout(binding = 6) uniform sampler2D motionMap;


#define MOTION_BLUR_SAMPLES 4.0
#define MOTION_SCALE 1.0




void main() {

    vec4 color = texture(sourceTexture, fs_in.texCoord).rgba;
    
    
    // Motion blur
    vec2 motion = texture(motionMap, fs_in.texCoord).xy;
    motion     *= MOTION_SCALE;
    
    vec4 avgColor = color;
    for(int i = 0; i < MOTION_BLUR_SAMPLES; ++i)
    {
        vec2 offset = motion * (float(i) / float(MOTION_BLUR_SAMPLES - 1) - 0.5);
        avgColor += texture(sourceTexture, fs_in.texCoord + offset);
    }
    avgColor /= MOTION_BLUR_SAMPLES;
    color = avgColor;
    
    
    
    // Bloom
    const float strength = 1.0;
    vec4 bloomHalfthSample = texture(bloomHalfth, fs_in.texCoord) * strength;
    vec4 bloomQuarterSample = texture(bloomQuarter, fs_in.texCoord) * strength * 0.75;
    vec4 bloomEigthSample = texture(bloomEigth, fs_in.texCoord) * strength * 0.5;
    vec4 bloomSixteenthSample = texture(bloomSixteenth, fs_in.texCoord) * strength * 0.25;
    vec4 bloom = (bloomHalfthSample + bloomQuarterSample + bloomEigthSample + bloomSixteenthSample);
    
    color += bloom;
    

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
	//fragColor = bloomQuarterSample;
}