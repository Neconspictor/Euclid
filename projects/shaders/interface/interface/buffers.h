#ifndef SHADER_INTERFACE_BUFFERS_H
#define SHADER_INTERFACE_BUFFERS_H
#include "interface/common_interface.h"
#include "interface/light_interface.h"
#include "interface/shadow/cascade_common.h"

#ifndef SHADER_CONSTANTS_UNIFORM_BUFFER_BINDING_POINT
#define SHADER_CONSTANTS_UNIFORM_BUFFER_BINDING_POINT 0
#endif

#ifndef OBJECT_SHADER_UNIFORM_BUFFER_BINDING_POINT
#define OBJECT_SHADER_UNIFORM_BUFFER_BINDING_POINT 1
#endif


#ifdef __cplusplus
namespace nex {
	struct ShaderConstants {
#else // GLSL 
layout(std140, binding = SHADER_CONSTANTS_UNIFORM_BUFFER_BINDING_POINT) uniform ShaderConstants {
#endif

	// camera and viewport
	NEX_MAT4		viewGPass;
	NEX_MAT4		invViewGPass;
	NEX_MAT4		invViewRotGPass; //mat3 used
	NEX_MAT4		projectionGPass;
	NEX_MAT4		invProjectionGPass;
	NEX_MAT4		invViewProjectionGPass;
	NEX_MAT4		prevViewProjectionGPass;

	NEX_VEC4		cameraPositionWS; //xyz used
	NEX_VEC4		viewport; //xy used

	NEX_VEC4		nearFarPlaneGPass; //xy used

	// voxels
	float			vct_VoxelRadianceDataSize;				// voxel half-extent in world space units
	float			vct_VoxelRadianceDataSize_rcp;			// 1.0 / voxel-half extent
	NEX_UINT		vct_VoxelRadianceDataRes;				// voxel grid resolution
	float			vct_VoxelRadianceDataRes_rcp;			// 1.0 / voxel grid resolution

	NEX_UINT		vct_VoxelRadianceDataMIPs;				// voxel grid mipmap count
	NEX_UINT		vct_VoxelRadianceNumCones;				// number of diffuse cones to trace
	float			vct_VoxelRadianceNumCones_rcp;			// 1.0 / number of diffuse cones to trace
	float			vct_VoxelRadianceRayStepSize;			// raymarch step size in voxel space units

	NEX_VEC4		vct_VoxelRadianceDataCenter;			// center of the voxel grid in world space units
	NEX_UINT		vct_VoxelRadianceReflectionsEnabled;	// are voxel gi reflections enabled or not   

	// atmospheric scattering
	float			atms_intensity; // the light intensity (strength) //TODO: should be stored in DirLight ?
	// phase (molecular reflection) factors
	float			atms_rayleigh_brightness;
	float			atms_mie_distribution;

	float			atms_mie_brightness;
	float			atms_spot_brightness;
	float			atms_surface_height; // in range [0.15, 1]
	NEX_UINT		atms_step_count; // defines the sample count for light scattering	

	float			atms_scatter_strength;
	float			atms_rayleigh_collection_power;
	float			atms_mie_collection_power;
	float			atms_rayleigh_strength;

	float			atms_mie_strength;


	// lighting and shadows

	float			ambientLightPower;
	float			shadowStrength;

	//Note: Arrays would be extended to a multiple to vec4 in glsl, 
	//Thus we only define it for the application and not for the shader
#ifdef __cplusplus
	float			_pad[1];
#endif

	DirLight		dirLight;
	CascadeData		cascadeData;

#ifdef __cplusplus
	};
#else // GLSL 
		} constants;
#endif


/**
 * Alignment size: 5 * 64 + 4 * 4 bytes = 272 bytes
 * Note: uniform buffer minimum max size: 16384 bytes
 *       -> space for 60 objects
 */
struct PerObjectMaterial {
	// matrices
	NEX_MAT4 model;
	NEX_MAT4 transform; //model view projection
	NEX_MAT4 prevTransform;
	NEX_MAT4 modelView;
	NEX_MAT4 normalMatrix; //mat3 used

	// For objects using reflection probes
	NEX_UINT probesUsed; //bool has 32 bit in glsl
	NEX_UINT diffuseReflectionProbeID;
	NEX_UINT specularReflectionProbeID;
	float _pad[1];
};

#ifndef __cplusplus //GLSL

#ifndef BUFFERS_DEFINE_OBJECT_BUFFER 
#define BUFFERS_DEFINE_OBJECT_BUFFER 1
#endif

	#if BUFFERS_DEFINE_OBJECT_BUFFER
		layout(std140, binding = OBJECT_SHADER_UNIFORM_BUFFER_BINDING_POINT) uniform ObjectShaderBuffer {
			PerObjectMaterial materials[];
		} objects;
	#endif

#endif



#ifdef __cplusplus
}
#endif 


#endif // SHADER_INTERFACE_BUFFERS_H