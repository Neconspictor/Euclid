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



#define MOTION_BLUR_SAMPLES 12.0
#define MOTION_SCALE 1.0

const vec4 sepiaColor = vec4(1.2, 1.0, 0.8, 1.0);


void main() {

    vec4 color = texture(sourceTexture, fs_in.texCoord);
    
    
    
    // Bloom
    const float strength = 0.5;
    vec3 bloomHalfthSample = clamp(texture(bloomHalfth, fs_in.texCoord).rgb, 0.0, 100.0) * strength;
    vec3 bloomQuarterSample = clamp(texture(bloomQuarter, fs_in.texCoord).rgb, 0.0, 100.0) * strength * 0.75;
    vec3 bloomEigthSample = clamp(texture(bloomEigth, fs_in.texCoord).rgb, 0.0, 100.0) * strength * 0.5;
    vec3 bloomSixteenthSample = clamp(texture(bloomSixteenth, fs_in.texCoord).rgb, 0.0, 100.0) * strength * 0.25;
    vec3 bloom = (bloomHalfthSample + bloomQuarterSample + bloomEigthSample + bloomSixteenthSample);
    
    color.rgb += bloom;
    
    // Ambient Occlusion
    //color.rgb *= texture(aoMap, fs_in.texCoord).r;
    
    
    // Motion blur
    vec2 motion = texture(motionMap, fs_in.texCoord).xy;
    //motion = vec2(0.1, 0.1);
    motion     *= MOTION_SCALE;
    
    vec4 avgColor = color;
    for(int i = 0; i < MOTION_BLUR_SAMPLES; ++i)
    {
        vec2 offset = motion * (float(i) / float(MOTION_BLUR_SAMPLES - 1) - 0.5);
        avgColor += texture(aoMap, fs_in.texCoord + offset).r * texture(sourceTexture, fs_in.texCoord + offset);
    }
    avgColor /= MOTION_BLUR_SAMPLES;
    //color = avgColor;
    
    
    // HDR tonemapping
    const float exposure = 1;
    color *= exposure;
    color = color / (color + vec4(1.0));
	
    // gamma correct
    const float gamma = 2.2f;
    color = pow(color, vec4(1.0/gamma)); 
    
    
    
    //Sepia
    //vec4 grayscale = vec4(dot(color, vec4(0.299, 0.587, 0.114, 1.0)));
    //color = mix(color, grayscale * sepiaColor, 0.3);
    
    //Vignette
    const float VignetteStrength = 10.0;
    const float power = 0.2;
    vec2 tuv = fs_in.texCoord * (vec2(1.0) - fs_in.texCoord.yx);
    float vign = tuv.x*tuv.y * VignetteStrength;
    vign = pow(vign, power);
    //color *= vign;
   

    fragColor = color;
	//fragColor = bloomQuarterSample;
}