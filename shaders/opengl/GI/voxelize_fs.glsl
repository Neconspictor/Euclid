#version 430 core

#ifndef C_UNIFORM_BUFFER_BINDING_POINT
#define C_UNIFORM_BUFFER_BINDING_POINT 0
#endif

#ifndef VOXEL_BUFFER_BINDING_POINT
#define VOXEL_BUFFER_BINDING_POINT 1
#endif

#ifndef VOXEL_BASE_SIZE
#define  VOXEL_BASE_SIZE 128.0
#endif

#ifndef VOXEL_DATE_SIZE_RCP
#define  VOXEL_DATE_SIZE_RCP 128.0
#endif

in GS_OUT {
    vec4 pos;
	vec2 texCoords;
	vec3 N;
	vec3 P;
} fs_in;


#include "interface/light_interface.h"
#include "GI/util.glsl"

layout(std140, binding = C_UNIFORM_BUFFER_BINDING_POINT) uniform Cbuffer {
    float       g_xFrame_VoxelRadianceDataSize;				// voxel half-extent in world space units
	float       g_xFrame_VoxelRadianceDataSize_rcp;			// 1.0 / voxel-half extent
    uint		g_xFrame_VoxelRadianceDataRes;				// voxel grid resolution
	float		g_xFrame_VoxelRadianceDataRes_rcp;			// 1.0 / voxel grid resolution
    
    uint		g_xFrame_VoxelRadianceDataMIPs;				// voxel grid mipmap count
	uint		g_xFrame_VoxelRadianceNumCones;				// number of diffuse cones to trace
	float		g_xFrame_VoxelRadianceNumCones_rcp;			// 1.0 / number of diffuse cones to trace
	float		g_xFrame_VoxelRadianceRayStepSize;			// raymarch step size in voxel space units
    
    vec4		g_xFrame_VoxelRadianceDataCenter;			// center of the voxel grid in world space units
	uint		g_xFrame_VoxelRadianceReflectionsEnabled;	// are voxel gi reflections enabled or not   
};


struct Material {
    layout(binding = 0) sampler2D albedoMap;
	layout(binding = 1) sampler2D aoMap;
	layout(binding = 2) sampler2D metallicMap;
	layout(binding = 3) sampler2D normalMap;
	layout(binding = 4) sampler2D roughnessMap;
};

uniform Material material;

uniform DirLight dirLight;

layout(std430, binding = VOXEL_BUFFER_BINDING_POINT) buffer VoxelBuffer {
    VoxelType voxels[];
};

void main()
{
    vec3 N = fs_in.N;
	vec3 P = fs_in.P;

	//vec3 diff = (P - g_xFrame_VoxelRadianceDataCenter.xyz) * g_xFrame_VoxelRadianceDataRes_rcp * g_xFrame_VoxelRadianceDataSize_rcp;
    vec3 diff = P * g_xFrame_VoxelRadianceDataRes_rcp * g_xFrame_VoxelRadianceDataSize_rcp;
    
    // Note: In original, y component uses -0,5f; but for opengl it should be +0.5f -> verify!
	vec3 uvw = diff * 0.5 + 0.5;
    
    if (!is_saturated(uvw)) {discard;}
	//{
        vec4 color = vec4(1.0);
        color = texture(material.albedoMap, fs_in.texCoords);
		//color.rgb = DEGAMMA(color.rgb); // Shouldn't be needed as we use sRGB textures which map automatically to linear space
        
        
        vec3 L = normalize(dirLight.directionWorld); // TODO: check if positive direction is needed!
        vec3 lightColor = dirLight.color.rgb * dirLight.power * max(dot(N, L), 0);
        
        //TODO: shadow maps!
        
        color.xyz *= lightColor;
        
        uint color_encoded = EncodeColor(color);
		uint normal_encoded = EncodeNormal(N);

		// output:
		//uvec3 writecoord = uvec3(floor(uvw * g_xFrame_VoxelRadianceDataRes));
        uvec3 writecoord = uvec3(floor(uvw * g_xFrame_VoxelRadianceDataRes));
		//uint id = flatten3D(writecoord, uvec3(g_xFrame_VoxelRadianceDataRes));
        uint id = flatten3D(writecoord, uvec3(g_xFrame_VoxelRadianceDataRes));
        atomicMax(voxels[id].colorMask, color_encoded);
		atomicMax(voxels[id].normalMask, normal_encoded);
        
        //voxels[id].colorMask = 10;
        //voxels[id].normalMask = 10;
        
        //voxels[writecoord.x].colorMask = 10;
        //voxels[writecoord.y].normalMask = 10;
        //voxels[writecoord.z].normalMask = 11;
        
        //voxels[1].colorMask = id;
        //atomicAdd(voxels[1].normalMask, 1);
    //}
}