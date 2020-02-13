#version 460 core

#ifndef LOCAL_SIZE_X
#define LOCAL_SIZE_X 256
#endif

#ifndef VOXEL_BUFFER_BINDING_POINT
#define VOXEL_BUFFER_BINDING_POINT 0
#endif

#include "interface/light_interface.h"

layout (local_size_x = 1, local_size_y = 1, local_size_z = 1) in;

#include "util/compute_util.glsl"
#include "GI/util.glsl"
#include "interface/buffers.h"


layout(std430, binding = VOXEL_BUFFER_BINDING_POINT) buffer VoxelBuffer {
    VoxelType voxels[];
};

layout (rgba32f, binding = 0) uniform image3D voxelImage;


void main()
{
    const uint globalInvocationIndex = getGlobalInvocationIndex();
    VoxelType voxel = voxels[globalInvocationIndex];
    vec4 albedo = DecodeColor(voxel.colorMask);
    imageStore(voxelImage, ivec3(gl_GlobalInvocationID) , albedo);
}