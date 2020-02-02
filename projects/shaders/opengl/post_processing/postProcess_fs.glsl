#version 460 core

in VS_OUT {
    vec2 texCoord;
} fs_in;

out vec4 fragColor;

layout(binding = 0) uniform sampler2D sourceTexture;
layout(binding = 1) uniform sampler2D bloomHalfth;
layout(binding = 2) uniform sampler2D bloomQuarter;
layout(binding = 3) uniform sampler2D bloomEigth;
layout(binding = 4) uniform sampler2D bloomSixteenth;
layout(binding = 5) uniform sampler2D motionMap;



#define MOTION_BLUR_SAMPLES 12.0
#define MOTION_SCALE 0.4

const vec4 sepiaColor = vec4(1.2, 1.0, 0.8, 1.0);
const float gamma = 2.2f;


#include "util/util.glsl"


vec3 Uncharted2ToneMapping(vec3 color)
{
	float A = 0.15;
	float B = 0.50;
	float C = 0.10;
	float D = 0.20;
	float E = 0.02;
	float F = 0.30;
	float W = 11.2;
	float exposure = 2.;
	color *= exposure;
	color = ((color * (A * color + C * B) + D * E) / (color * (A * color + B) + D * F)) - E / F;
	float white = ((W * (A * W + C * B) + D * E) / (W * (A * W + B) + D * F)) - E / F;
	color /= white;
	color = pow(color, vec3(1. / gamma));
	return color;
}

vec3 linearToneMapping(vec3 color)
{
	float exposure = 1.;
	color = clamp(exposure * color, 0., 1.);
	color = pow(color, vec3(1. / gamma));
	return color;
}

vec3 simpleReinhardToneMapping(vec3 color)
{
	float exposure = 1.5;
	color *= exposure/(1. + color / exposure);
	color = pow(color, vec3(1. / gamma));
	return color;
}

vec3 lumaBasedReinhardToneMapping(vec3 color)
{
	float luma = dot(color, vec3(0.2126, 0.7152, 0.0722));
	float toneMappedLuma = luma / (1. + luma);
	color *= toneMappedLuma / luma;
	color = pow(color, vec3(1. / gamma));
	return color;
}

vec3 whitePreservingLumaBasedReinhardToneMapping(vec3 color)
{
	float white = 2.;
	float luma = dot(color, vec3(0.2126, 0.7152, 0.0722));
	float toneMappedLuma = luma * (1. + luma / (white*white)) / (1. + luma);
	color *= toneMappedLuma / luma;
	color = pow(color, vec3(1. / gamma));
	return color;
}

vec3 RomBinDaHouseToneMapping(vec3 color)
{
    color = exp( -1.0 / ( 2.72*color + 0.15 ) );
	color = pow(color, vec3(1. / gamma));
	return color;
}

vec3 filmicToneMapping(vec3 color)
{
	color = max(vec3(0.), color - vec3(0.004));
	color = (color * (6.2 * color + .5)) / (color * (6.2 * color + 1.7) + 0.06);
	return color;
}


void main() {

    vec4 color = texture(sourceTexture, fs_in.texCoord);
    
    
    
    // Bloom
    const float strength = 1.0;
    vec4 bloomHalfthSample = clamp(texture(bloomHalfth, fs_in.texCoord).rgba, 0.0, 100.0) * strength;
    vec4 bloomQuarterSample = clamp(texture(bloomQuarter, fs_in.texCoord).rgba, 0.0, 100.0) * strength * 0.75;
    vec4 bloomEigthSample = clamp(texture(bloomEigth, fs_in.texCoord).rgba, 0.0, 100.0) * strength * 0.5;
    vec4 bloomSixteenthSample = clamp(texture(bloomSixteenth, fs_in.texCoord).rgba, 0.0, 100.0) * strength * 0.25;
    vec4 bloom = (bloomHalfthSample + bloomQuarterSample + bloomEigthSample + bloomSixteenthSample);
    
	bloom.rgb = bloom.rgb * pow(2.0, rgb2luma(bloom) - 1.0);
	
    color.rgb += bloom.rgb * bloom.a;
    
    // Motion blur
    vec2 motion = texture(motionMap, fs_in.texCoord).xy;
	
	vec2 texSize = textureSize(sourceTexture, 0);
	float yScale = texSize.x / texSize.y;
	
    //motion = vec2(0.1, 0.1);
    motion     = MOTION_SCALE * vec2(motion.x, motion.y);
	//motion = clamp(motion, - vec2(MOTION_BLUR_SAMPLES), vec2(MOTION_BLUR_SAMPLES));
    
    vec4 avgColor = color;
    for(int i = 1; i < MOTION_BLUR_SAMPLES; ++i)
    {
        vec2 offset = motion * (float(i) / float(MOTION_BLUR_SAMPLES)); //- 0.5
        avgColor += texture(sourceTexture, fs_in.texCoord + offset) + vec4(bloom.rgb * bloom.a, 0);
    }
    avgColor /= (MOTION_BLUR_SAMPLES);
    color = avgColor;
    
    
    // HDR tonemapping
    const float exposure = 1;
    color.rgb *= exposure;
    //color.rgb = color.rgb / (color.rgb + vec3(1.0));
    //color.rgb = pow(color.rgb, vec3(1.0/gamma));
    
    vec3 beforeToneMapping = color.rgb;
    
    
    //simple tonemapping
    vec3 firstTone = beforeToneMapping / (beforeToneMapping + vec3(1.0));
    firstTone = pow(firstTone, vec3(1.0/gamma));
    vec3 secondTone = whitePreservingLumaBasedReinhardToneMapping(beforeToneMapping);
    vec3 thirdTone = Uncharted2ToneMapping(beforeToneMapping);
    color.rgb = mix(firstTone, secondTone, 0.3);
    color.rgb = mix(color.rgb, thirdTone, 0.3);
    
    
    
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