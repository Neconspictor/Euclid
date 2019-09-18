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

struct VoxelType
{
	uint colorMask;
	uint normalMask;
};

const float HDR_RANGE = 10.0f;

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

uint EncodeColor(in vec4 color);
vec4 DecodeColor(in uint colorMask);
uint EncodeNormal(in vec3 normal);
vec3 DecodeNormal(in uint normalMask);

bool is_saturated(float a) { return a == clamp(a, 0.0, 1.0); }
bool is_saturated(vec2 a) { return is_saturated(a.x) && is_saturated(a.y); }
bool is_saturated(vec3 a) { return is_saturated(a.x) && is_saturated(a.y) && is_saturated(a.z); }
bool is_saturated(vec4 a) { return is_saturated(a.x) && is_saturated(a.y) && is_saturated(a.z) && is_saturated(a.w); }

// 3D array index to flattened 1D array index
uint flatten3D(uvec3 coord, uvec3 dim)
{
	return (coord.z * dim.x * dim.y) + (coord.y * dim.x) + coord.x;
}
// flattened array index to 3D array index
uvec3 unflatten3D(uint idx, uvec3 dim)
{
	const uint z = idx / (dim.x * dim.y);
	idx -= (z * dim.x * dim.y);
	const uint y = idx / dim.x;
	const uint x = idx % dim.x;
	return  uvec3(x, y, z);
}

void main()
{
    vec3 N = fs_in.N;
	vec3 P = fs_in.P;

	//vec3 diff = (P - g_xFrame_VoxelRadianceDataCenter.xyz) * g_xFrame_VoxelRadianceDataRes_rcp * g_xFrame_VoxelRadianceDataSize_rcp;
    vec3 diff = P * 1.0/VOXEL_BASE_SIZE * VOXEL_DATE_SIZE_RCP;
    
    // Note: In original, y component uses -0,5f; but for opengl it should be +0.5f -> verify!
	vec3 uvw = diff * 0.5 + 0.5;
    
    if (!is_saturated(uvw)) {discard;}
	//{
        vec4 color = vec4(1.0);
		//color = texture(material.albedoMap, fs_in.texCoords);
		//color.rgb = DEGAMMA(color.rgb); // Shouldn't be needed as we use sRGB textures which map automatically to linear space
        
        
        vec3 L = normalize(dirLight.directionWorld); // TODO: check if positive direction is needed!
        vec3 lightColor = dirLight.color.rgb * dirLight.power * max(dot(N, L), 0);
        
        //TODO: shadow maps!
        
        color.xyz *= lightColor;
        
        uint color_encoded = EncodeColor(color);
		uint normal_encoded = EncodeNormal(N);

		// output:
		//uvec3 writecoord = uvec3(floor(uvw * g_xFrame_VoxelRadianceDataRes));
        uvec3 writecoord = uvec3(floor(uvw * VOXEL_BASE_SIZE));
		//uint id = flatten3D(writecoord, uvec3(g_xFrame_VoxelRadianceDataRes));
        uint id = flatten3D(writecoord, uvec3(VOXEL_BASE_SIZE));
        atomicAdd(voxels[id].colorMask, color_encoded);
		atomicAdd(voxels[id].normalMask, normal_encoded);
        
        //voxels[id].colorMask = 10;
        //voxels[id].normalMask = 10;
        
        //voxels[writecoord.x].colorMask = 10;
        //voxels[writecoord.y].normalMask = 10;
        //voxels[writecoord.z].normalMask = 11;
        
        //voxels[1].colorMask = id;
        //atomicAdd(voxels[1].normalMask, 1);
    //}
}  


// Encode HDR color to a 32 bit uint
// Alpha is 1 bit + 7 bit HDR remapping
uint EncodeColor(in vec4 color)
{
	// normalize color to LDR
	float hdr = length(color.rgb);
	color.rgb /= hdr;

	// encode LDR color and HDR range
	uvec3 iColor = uvec3(color.rgb * 255.0f);
	uint iHDR = uint(clamp(hdr / HDR_RANGE, 0.0, 1.0) * 127);
	uint colorMask = (iHDR << 24u) | (iColor.r << 16u) | (iColor.g << 8u) | iColor.b;

	// encode alpha into highest bit
	uint iAlpha = (color.a > 0 ? 1u : 0u);
	colorMask |= iAlpha << 31u;

	return colorMask;
}

// Decode 32 bit uint into HDR color with 1 bit alpha
vec4 DecodeColor(in uint colorMask)
{
	float hdr;
	vec4 color;

	hdr = (colorMask >> 24u) & 0x0000007f;
	color.r = (colorMask >> 16u) & 0x000000ff;
	color.g = (colorMask >> 8u) & 0x000000ff;
	color.b = colorMask & 0x000000ff;

	hdr /= 127.0f;
	color.rgb /= 255.0f;

	color.rgb *= hdr * HDR_RANGE;

	color.a = (colorMask >> 31u) & 0x00000001;

	return color;
}

// Encode specified normal (normalized) into an unsigned integer. Each axis of
// the normal is encoded into 9 bits (1 for the sign/ 8 for the value).
uint EncodeNormal(in vec3 normal)
{
	ivec3 iNormal = ivec3(normal*255.0f);
	uvec3 iNormalSigns;
	iNormalSigns.x = (iNormal.x >> 5) & 0x04000000;
	iNormalSigns.y = (iNormal.y >> 14) & 0x00020000;
	iNormalSigns.z = (iNormal.z >> 23) & 0x00000100;
	iNormal = abs(iNormal);
	uint normalMask = iNormalSigns.x | (iNormal.x << 18) | iNormalSigns.y | (iNormal.y << 9) | iNormalSigns.z | iNormal.z;
	return normalMask;
}

// Decode specified mask into a float3 normal (normalized).
vec3 DecodeNormal(in uint normalMask)
{
	ivec3 iNormal;
	iNormal.x = int((normalMask >> 18) & 0x000000ff);
	iNormal.y = int((normalMask >> 9) & 0x000000ff);
	iNormal.z = int(normalMask & 0x000000ff);
	ivec3 iNormalSigns;
	iNormalSigns.x = int((normalMask >> 25) & 0x00000002);
	iNormalSigns.y = int((normalMask >> 16) & 0x00000002);
	iNormalSigns.z = int((normalMask >> 7) & 0x00000002);
	iNormalSigns = 1 - iNormalSigns;
	vec3 normal = vec3(iNormal) / 255.0f;
	normal *= iNormalSigns;
	return normal;
}