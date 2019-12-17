#version 460 core

#ifndef C_UNIFORM_BUFFER_BINDING_POINT
#define C_UNIFORM_BUFFER_BINDING_POINT 0
#endif

#ifndef VOXEL_BUFFER_BINDING_POINT
#define VOXEL_BUFFER_BINDING_POINT 0
#endif

out VS_OUT {
    vec3 positionWS;
    vec4 color;
} vs_out;


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

#include "GI/util.glsl"


layout(std430, binding = VOXEL_BUFFER_BINDING_POINT) buffer VoxelBuffer {
    VoxelType voxels[];
};

layout(binding = 0) uniform sampler3D voxelTexture;
uniform float mipMap;

void main()
{
    //VoxelType voxel = voxels[gl_VertexID];
    //vs_out.color = DecodeColor(voxel.colorMask); 
    uvec3 id = unflatten3D(gl_VertexID, uvec3(g_xFrame_VoxelRadianceDataRes));
    vec3 texCoord = vec3(id) * g_xFrame_VoxelRadianceDataRes_rcp;  
    vs_out.color = textureLod(voxelTexture, texCoord, mipMap);
    vs_out.positionWS = (2.0 * texCoord - 1.0) * g_xFrame_VoxelRadianceDataSize * g_xFrame_VoxelRadianceDataRes;
    gl_Position = vec4(vs_out.positionWS, 1.0f); 
}