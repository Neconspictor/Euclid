#version 430 core

#ifndef LOCAL_SIZE_X
#define LOCAL_SIZE_X 256
#endif

#ifndef C_UNIFORM_BUFFER_BINDING_POINT
#define C_UNIFORM_BUFFER_BINDING_POINT 0
#endif

#ifndef VOXEL_BUFFER_BINDING_POINT
#define VOXEL_BUFFER_BINDING_POINT 0
#endif

layout (local_size_x = 1, local_size_y = 1, local_size_z = 1) in;

#include "util/compute_util.glsl"
#include "GI/util.glsl"

layout(std430, binding = VOXEL_BUFFER_BINDING_POINT) buffer VoxelBuffer {
    VoxelType voxels[];
};

layout (rgba16f, binding = 0) uniform image3D voxelImage;

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


void main()
{
    const uint globalInvocationIndex = getGlobalInvocationIndex();
    VoxelType voxel = voxels[globalInvocationIndex];
    vec4 color = DecodeColor(voxel.colorMask);
    imageStore(voxelImage, ivec3(gl_GlobalInvocationID) , color);
    //imageStore(voxelImage, ivec3(0,0,0) , vec4(0.5,0.5, 0.5,1.0));
    
    /*if (color.a > 0 && color.r > 0.4) {
        imageStore(voxelImage, ivec3(0,0,0) , color);
    }*/
}