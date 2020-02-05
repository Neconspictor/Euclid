#ifndef SHADER_INTERFACE_VOXEL_INTERFACE_H
#define SHADER_INTERFACE_VOXEL_INTERFACE_H
#include "interface/common_interface.h"

#ifdef __cplusplus
namespace nex {
#endif

struct VoxelConstants 
{
	float       g_xFrame_VoxelRadianceDataSize;				// voxel half-extent in world space units
	float       g_xFrame_VoxelRadianceDataSize_rcp;			// 1.0 / voxel-half extent +++
	NEX_UINT	g_xFrame_VoxelRadianceDataRes;				// voxel grid resolution +++
	float		g_xFrame_VoxelRadianceDataRes_rcp;			// 1.0 / voxel grid resolution +++

	NEX_UINT	g_xFrame_VoxelRadianceDataMIPs;				// voxel grid mipmap count
	NEX_UINT	g_xFrame_VoxelRadianceNumCones;				// number of diffuse cones to trace
	float		g_xFrame_VoxelRadianceNumCones_rcp;			// 1.0 / number of diffuse cones to trace
	float		g_xFrame_VoxelRadianceRayStepSize;			// raymarch step size in voxel space units

	NEX_VEC4	g_xFrame_VoxelRadianceDataCenter;			// center of the voxel grid in world space units +++
	NEX_UINT	g_xFrame_VoxelRadianceReflectionsEnabled;	// are voxel gi reflections enabled or not
	
	#ifdef __cplusplus
		float _pad[3];
	#endif
};

#ifdef __cplusplus
}
#endif

#endif // SHADER_INTERFACE_VOXEL_INTERFACE_H