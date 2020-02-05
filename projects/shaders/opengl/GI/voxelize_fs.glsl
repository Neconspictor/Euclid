#version 460 core

#include "interface/buffers.h"

#ifndef VOXEL_BUFFER_BINDING_POINT
#define VOXEL_BUFFER_BINDING_POINT 1
#endif

#ifndef VOXEL_BASE_SIZE
#define  VOXEL_BASE_SIZE 128.0
#endif

#ifndef VOXEL_DATE_SIZE_RCP
#define  VOXEL_DATE_SIZE_RCP 128.0
#endif

#ifndef VOXEL_LIGHTING_WHILE_VOXELIZING
#define VOXEL_LIGHTING_WHILE_VOXELIZING 1
#endif

in GS_OUT {
    vec4 pos;
	vec2 texCoords;
	vec3 N;
	vec3 P;
} fs_in;


#include "GI/util.glsl"
#include "interface/light_interface.h"


struct Material {
    layout(binding = 0) sampler2D albedoMap;
	layout(binding = 1) sampler2D aoMap;
	layout(binding = 2) sampler2D metallicMap;
	layout(binding = 3) sampler2D normalMap;
	layout(binding = 4) sampler2D roughnessMap;
};

uniform Material material;

layout(std430, binding = VOXEL_BUFFER_BINDING_POINT) buffer VoxelBuffer {
    VoxelType voxels[];
};

#if VOXEL_LIGHTING_WHILE_VOXELIZING

    uniform DirLight dirLight;

    #ifndef SHADOW_DEPTH_MAP_BINDING_POINT
    #define SHADOW_DEPTH_MAP_BINDING_POINT 5
    #endif
    #include "shadow/shadow_map.glsl"
  
#endif    


void main()
{
    vec3 N = fs_in.N;
	vec3 P = fs_in.P;

	//vec3 diff = (P - g_xFrame_VoxelRadianceDataCenter.xyz) * g_xFrame_VoxelRadianceDataRes_rcp * g_xFrame_VoxelRadianceDataSize_rcp;
    vec3 diff = P * constants.voxels.g_xFrame_VoxelRadianceDataRes_rcp * constants.voxels.g_xFrame_VoxelRadianceDataSize_rcp;
    
    // Note: In original, y component uses -0,5f; but for opengl it should be +0.5f -> verify!
	vec3 uvw = diff * 0.5 + 0.5;
    
    if (!is_saturated(uvw)) {discard;}
    
    #if VOXEL_LIGHTING_WHILE_VOXELIZING
        vec4 albedo = texture(material.albedoMap, fs_in.texCoords);
        vec3 L = normalize(dirLight.directionWorld.xyz); // TODO: check if positive direction is needed!
        vec3 lightColor = dirLight.color.rgb * dirLight.power * max(dot(N, L), 0);
        float shadow = computeShadow(L, N, P);
        
        if (L.y > 0) shadow = 0.0;
        
        vec4 color = vec4(albedo.rgb * lightColor * shadow, albedo.a);
    #else
        vec4 color = texture(material.albedoMap, fs_in.texCoords);
    #endif


    uint color_encoded = EncodeColor(color);
    uint normal_encoded = EncodeNormal(N);

    // output:
    uvec3 writecoord = uvec3(floor(uvw * constants.voxels.g_xFrame_VoxelRadianceDataRes));
    uint id = flatten3D(writecoord, uvec3(constants.voxels.g_xFrame_VoxelRadianceDataRes));
    atomicMax(voxels[id].colorMask, color_encoded);
    atomicMax(voxels[id].normalMask, normal_encoded);
	
	//atomicMax(voxels[0].colorMask, 5);
    //atomicMax(voxels[0].normalMask, 23);
}