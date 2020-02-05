#version 460 core


#ifndef VOXEL_BUFFER_BINDING_POINT
#define VOXEL_BUFFER_BINDING_POINT 0
#endif

out VS_OUT {
    vec3 positionWS;
    vec4 color;
} vs_out;


#include "GI/util.glsl"


layout(std430, binding = VOXEL_BUFFER_BINDING_POINT) buffer VoxelBuffer {
    VoxelType voxels[];
};

layout(binding = 0) uniform sampler3D voxelTexture;
uniform float mipMap;

#include "interface/buffers.h"

void main()
{
    //VoxelType voxel = voxels[gl_VertexID];
    //vs_out.color = DecodeColor(voxel.colorMask); 
    uvec3 id = unflatten3D(gl_VertexID, uvec3(constants.voxels.g_xFrame_VoxelRadianceDataRes));
    vec3 texCoord = vec3(id) * constants.voxels.g_xFrame_VoxelRadianceDataRes_rcp;  
    vs_out.color = textureLod(voxelTexture, texCoord, mipMap);
    vs_out.positionWS = (2.0 * texCoord - 1.0) * constants.voxels.g_xFrame_VoxelRadianceDataSize * constants.voxels.g_xFrame_VoxelRadianceDataRes;
    gl_Position = vec4(vs_out.positionWS, 1.0f); 
}