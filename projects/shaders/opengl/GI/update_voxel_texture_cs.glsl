#version 460 core

#ifndef LOCAL_SIZE_X
#define LOCAL_SIZE_X 256
#endif

#ifndef VOXEL_BUFFER_BINDING_POINT
#define VOXEL_BUFFER_BINDING_POINT 0
#endif

#ifndef VOXEL_LIGHTING_WHILE_VOXELIZING
#define VOXEL_LIGHTING_WHILE_VOXELIZING 1
#endif

#if !VOXEL_LIGHTING_WHILE_VOXELIZING

    #ifndef SHADOW_DEPTH_MAP_BINDING_POINT
    #define SHADOW_DEPTH_MAP_BINDING_POINT 1
    #endif
    #include "shadow/shadow_map.glsl"
#endif

#include "interface/light_interface.h"

#if !VOXEL_LIGHTING_WHILE_VOXELIZING
	uniform DirLight dirLight;
#endif

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
    
    //if (voxel.colorMask == 0) return;
    
    
    
    vec4 albedo = DecodeColor(voxel.colorMask);
    vec3 N = normalize(DecodeNormal(voxel.normalMask));
    
    #if VOXEL_LIGHTING_WHILE_VOXELIZING
        imageStore(voxelImage, ivec3(gl_GlobalInvocationID) , albedo);
    #else
    
        /*  get position of the voxel:
            diff = P * g_xFrame_VoxelRadianceDataRes_rcp * g_xFrame_VoxelRadianceDataSize_rcp;
            uvw = (diff * 0.5 + 0.5);
            voxelCoordinate = uvw * g_xFrame_VoxelRadianceDataRes;
        */
        vec3 uvw = gl_GlobalInvocationID * constants.voxels.g_xFrame_VoxelRadianceDataRes_rcp;
        vec3 diff = 2.0 * uvw - 1.0;
        vec3 P = diff * float(constants.voxels.g_xFrame_VoxelRadianceDataRes) * constants.voxels.g_xFrame_VoxelRadianceDataSize;
    
        vec3 L = normalize(dirLight.directionWorld.xyz); // TODO: check if positive direction is needed!
        vec3 lightColor = dirLight.color.rgb * dirLight.power * max(dot(N, L), 0);
        float shadow = computeShadow(L, N, P);
        //if (L.y < 0) shadow = 0.0;
        vec4 color = vec4(albedo.rgb * lightColor * shadow, albedo.a); //* lightColor * shadow    albedo.rgb * lightColor * shadow
        imageStore(voxelImage, ivec3(gl_GlobalInvocationID) , color);
    #endif
}