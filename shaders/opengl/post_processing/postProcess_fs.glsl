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

layout(binding = 7) uniform sampler2D oceanHeightMap;
layout(binding = 8) uniform sampler2D depthMap;

layout(binding = 9) uniform sampler2D oceanDX;
layout(binding = 10) uniform sampler2D oceanDZ;


uniform mat4 inverseViewProjMatrix_GPass;
uniform mat4 inverseModelMatrix_Ocean;
uniform float oceanTileSize;
//uniform mat4 model_Ocean;



#define MOTION_BLUR_SAMPLES 12.0
#define MOTION_SCALE 1.0

const vec4 sepiaColor = vec4(1.2, 1.0, 0.8, 1.0);

vec3 computeWorldPositionFromDepth(in vec2 texCoord, in float depth) {
  vec4 clipSpaceLocation;
  clipSpaceLocation.xy = texCoord * 2.0f - 1.0f;
  clipSpaceLocation.z = depth * 2.0f - 1.0f;
  clipSpaceLocation.w = 1.0f;
  vec4 homogenousLocation = inverseViewProjMatrix_GPass * clipSpaceLocation;
  return homogenousLocation.xyz / homogenousLocation.w;
};




void main() {

    vec4 color = texture(sourceTexture, fs_in.texCoord).rgba;
    
    
    
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
    
    
    // check if under water
    {
        vec3 worldPosition = computeWorldPositionFromDepth(fs_in.texCoord, texture(depthMap, fs_in.texCoord).r);
        vec4 oceanPosition = inverseModelMatrix_Ocean * vec4(worldPosition, 1.0);
        
        
        
        
        const vec2 tileFactor = oceanPosition.xy / uint(oceanTileSize);
        vec2 oceanUV = oceanPosition.xz - tileFactor * oceanTileSize;
        oceanUV /= oceanTileSize; // normalize to range [0,1]
        const float mLambda = -1.0;
        
        oceanPosition.x += mLambda * texture(oceanDX, oceanUV).x;
        oceanPosition.z += mLambda * texture(oceanDZ, oceanUV).x;
        
        oceanUV = oceanPosition.xz - tileFactor * oceanTileSize;
        oceanUV /= oceanTileSize; 
        
        float oceanHeight = texture(oceanHeightMap, oceanUV).r;
        
        //vec4 oceanReferenceWS = model_Ocean * vec4(oceanUV, oceanHeight, 1.0);
        
        // Is fragment below water?
        if (oceanHeight > (oceanPosition.y - 5.0) && false) 
        {
            vec2 texSize = textureSize(sourceTexture, 0);
            vec4 avgColor = color;
            for(int i = 1; i < 9; ++i)
            {
                for (int j = 1; j < 9; ++j) {
                    vec2 offset = (1.0 / texSize) * (vec2(i,j) - 0.5);
                    vec2 uv = fs_in.texCoord + offset;
                    color += texture(sourceTexture, uv);
                }   
                
            }
            color /= 64.0;
        
        
            vec4 diffuse_color  = vec4(0.0, 0.65, 0.75, color.a);
            diffuse_color.rgb = color.rgb * diffuse_color.rgb + 0.1 * diffuse_color.rgb;
            color = mix(color, diffuse_color, 0.5);
        }
        
       // if (oceanHeight == oceanPosition.y) {
       //     color = color + vec4(1.0, 0.0, 0.0, 0.0);
       // }
    
    }
    
    
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