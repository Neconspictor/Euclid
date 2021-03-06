#version 460 core

in VS_OUT {
    vec2 texCoord;
} fs_in;

out vec4 fragColor;
out vec4 luminanceColor;


layout(binding = 0) uniform sampler2D colorMap;
layout(binding = 1) uniform sampler2D oceanHeightMap;
layout(binding = 2) uniform sampler2D depthMap;
layout(binding = 3) uniform usampler2D stencilMap;
//layout(binding = 4) uniform sampler2D oceanDX;
//layout(binding = 5) uniform sampler2D oceanDZ;
layout(binding = 4) uniform isampler1D oceanMinHeightMap;
layout(binding = 5) uniform isampler1D oceanMaxHeightMap;

uniform mat4 inverseViewProjMatrix_GPass;
uniform mat4 inverseModelMatrix_Ocean;
uniform float oceanTileSize;
uniform vec3 cameraPosition;
uniform float murk;
uniform float waterLevel;

#include "util/depth_util.glsl"

vec2 getWaterUV(in vec3 oceanPos) {
    
    const float lambda = -1.0;
    const float pointCount = 129;
    const float waveLength = 128;
    const float N = 128;
    vec2 mDX = vec2(0);// texture(oceanDX, oceanPos.xz).xy;
    vec2 mDZ = vec2(0);// texture(oceanDZ, oceanPos.xz).xy;
    
    
    
    vec2 oceanUV = vec2(oceanPos.x + lambda * mDX.x, oceanPos.z + lambda * mDZ.x);
    oceanUV = oceanPos.xz;
    
    
    vec2 uv;
    
    //(x - mPointCount / 2.0f) * mWaveLength / (float)mN,
    uv.x = oceanUV.x * N / waveLength;
    uv.x += pointCount / 2.0;
    
    //getZValue((z - mPointCount / 2.0f) * mWaveLength / (float)mN)
    
    uv.y = -oceanUV.y;
    uv.y = uv.y * N / waveLength;
    
    uv.y += pointCount / 2.0;
    
    uv.x = float(uint(uv.x) % uint(N)) / pointCount;
    uv.y = float(uint(uv.y) % uint(N)) / pointCount;
    
    return uv; 
}

float getLuma(in vec3 rgb) {
    return 0.299 * rgb.r + 0.587 * rgb.g + 0.114 * rgb.b; 
}


void main() {

    vec4 color = texture(colorMap, fs_in.texCoord);
    vec3 worldPosition = reconstructPositionFromDepth(inverseViewProjMatrix_GPass, fs_in.texCoord, texture(depthMap, fs_in.texCoord).r);        
    vec4 oceanPosition = inverseModelMatrix_Ocean * vec4(worldPosition, 1.0);
   
    const vec2 tileFactor = oceanPosition.xz / uint(8);
    vec2 oceanUV = oceanPosition.xz - tileFactor * 8;
    oceanUV /= 8; // normalize to range [0,1]
    //const float mLambda = -1.0;
    
   //oceanPosition.x -=  texture(oceanDX, oceanUV).x;
   //oceanPosition.z -= texture(oceanDZ, oceanUV).x;
    
    
    vec3 xyz = oceanPosition.xyz;
    
    //xyz.x -= texture(oceanDX, oceanUV).x;
    //xyz.z -= texture(oceanDZ, oceanUV).x;
    
    //vec3 factor = xyz / uint(8);
    //xyz = xyz - factor * uint(8);
    
    
    
    vec2 waterUV = getWaterUV(xyz);
    //waterUV /= oceanTileSize;
   //waterUV = oceanUV;
    //waterUV.y *= -1.0;
    //waterUV.y *= -1.0;
    
    float oceanHeight = texture(oceanHeightMap, waterUV).r;
    uint oceanStencil = texture(stencilMap, fs_in.texCoord).r;
    
    float minOceanHeight = intBitsToFloat(texture(oceanMinHeightMap, fs_in.texCoord.x).r);
    float maxOceanHeight = intBitsToFloat(texture(oceanMaxHeightMap, fs_in.texCoord.x).r);
    vec2 texSize = textureSize(depthMap, 0);
    float reference = fs_in.texCoord.y * texSize.y;
    //float yCoordF = minOceanHeight / texSize.y;
    //vec4 oceanReferenceWS = model_Ocean * vec4(oceanUV, oceanHeight, 1.0);
    
    bool underWater = minOceanHeight != 1000000000;
    bool heightCompare = oceanHeight > oceanPosition.y;
    
    bool compare1 = maxOceanHeight > fs_in.texCoord.y;
    bool compare2 = minOceanHeight > fs_in.texCoord.y;
    bool compare3 = abs(minOceanHeight - fs_in.texCoord.y) < 0.1;
    bool compare4 = abs(maxOceanHeight - fs_in.texCoord.y) < 0.1;
    bool compare5 = compare3 ||compare4;
    bool compare6 = maxOceanHeight > fs_in.texCoord.y && (minOceanHeight < fs_in.texCoord.y);
    bool compare7 = worldPosition.y < 3.0;
    bool compare8 = oceanHeight > oceanPosition.y;
    bool compare9 = cameraPosition.y < 3.1; //&& !underWater;
	bool compare10 = minOceanHeight >= maxOceanHeight;
	bool compare11 = max(maxOceanHeight, minOceanHeight) > fs_in.texCoord.y;
	
	if (cameraPosition.y > waterLevel + 0.3) {
		discard;
	}
	
	//(worldPosition.y < 3.5)
   
   //oceanStencil == 0 && 
    // Is fragment below water?
    if ((compare11 || (cameraPosition.y <waterLevel - 0.3)))//(compare1 || compare10) && (cameraPosition.y < 4.0)) //|| coordCompare2 && underWater 
    {
        
        vec4 avgColor = color;
        for(int i = 1; i < 9; ++i)
        {
            for (int j = 1; j < 9; ++j) {
                vec2 offset = (1.0 / texSize) * (vec2(i,j) - 0.5);
                vec2 uv = fs_in.texCoord + offset;
                color += texture(colorMap, uv);
            }   
            
        }
        color /= 64.0;
    
        float litLuma = clamp(getLuma(color.rgb), 0.0, 1.0);
		
		vec3 sunColor = vec3(1.0, 1.0, 1.0);//vec3(1.0);
		float sunScale = 3.0;
		vec3 waterColor = vec3(clamp(length(sunColor) / sunScale, 0, 1));
		
		vec3 bigDepthColor = vec3(0.00039, 0.000196, 0.0145);
		float visibleDistance = 3.0 / murk;
		vec3 extinction = vec3(4.5, 25.0, 40.0);
		float A = length(worldPosition - cameraPosition) * murk;
		float waterY = waterLevel;
		float D = abs(waterY - worldPosition.y);
		
		
		color.rgb = mix(color.rgb, bigDepthColor * waterColor, clamp(D / extinction, 0.0, 1.0));
		color.rgb = mix(color.rgb, bigDepthColor * waterColor, smoothstep(0.0, 1.0, smoothstep(0.0, visibleDistance, length(worldPosition - cameraPosition))));
		
        //color = mix(color, vec4(0.5, 0.7, 1.0, color.a), 0.1);
        //color.rgb *= litLuma * vec3(0.5, 0.7, 1.0);
        //color = mix(color, vec4(0.3, 0.5, 0.7, color.a), 0.5);
        //color.rgb *= litLuma * vec3(1.0, 0.9, 0.7);
        //color.rgb *= litLuma;
		
		
		//color = vec4(1,0,0,1);
		
    } else {
        discard;
    }
    
    fragColor = color;
    
   // if (oceanHeight == oceanPosition.y) {
   //     color = color + vec4(1.0, 0.0, 0.0, 0.0);
   // }
   
   luminanceColor = 0.5 * color;
}