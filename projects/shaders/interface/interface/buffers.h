#ifndef SHADER_INTERFACE_BUFFERS_H
#define SHADER_INTERFACE_BUFFERS_H
#include "interface/common_interface.h"
#include "interface/light_interface.h"
#include "interface/shadow/cascade_common.h"
#include "interface/GI/voxel_interface.h"

#ifndef SHADER_CONSTANTS_UNIFORM_BUFFER_BINDING_POINT
#define SHADER_CONSTANTS_UNIFORM_BUFFER_BINDING_POINT 7
#endif

#ifndef OBJECT_SHADER_UNIFORM_BUFFER_BINDING_POINT
#define OBJECT_SHADER_UNIFORM_BUFFER_BINDING_POINT 8
#endif

#ifndef SHADER_CONSTANTS_MATERIAL_BUFFER_BINDING_POINT
#define SHADER_CONSTANTS_MATERIAL_BUFFER_BINDING_POINT 9
#endif


#ifdef __cplusplus
namespace nex {
	struct ShaderConstants {
#else // GLSL 
layout(column_major, std140, binding = SHADER_CONSTANTS_UNIFORM_BUFFER_BINDING_POINT) uniform ShaderConstants {
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

	VoxelConstants voxels;

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
	float			_pad[2];
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
 */
struct PerObjectData {
	
	NEX_UINT perObjectMaterialID;
	#ifdef __cplusplus
	float _pad1[3];
	#endif 
	
	// matrices
	NEX_MAT4 model;
	NEX_MAT4 transform; //model view projection
	NEX_MAT4 prevTransform;
	NEX_MAT4 modelView;

#ifdef __cplusplus
	NEX_MAT4 normalMatrix; //mat3 where each column vector is extended to a vec4
#else 
	NEX_MAT3 normalMatrix;
#endif
	
};


/**
 * Alignment size: 8*4 + 9 * 4 * 4 + 2 * 16 = 32 + 144 + 32 = 208
 * Note: uniform buffer minimum max size: 16384 bytes
 *       -> space for 78 objects
 */

struct PerObjectMaterialData {
#ifdef __cplusplus
	// For objects using reflection probes
	int probesUsed = 1; //bool has 32 bit in glsl
	float probeInfluence = 1.0f;
	float emissionStrength = 1.0f;
	float _pad0;

	int coneTracingUsed = 0;
	float coneTracingInfluence = 1.0f;
	float _pad1[2];

#else
	// For objects using reflection probes
	NEX_IVEC4 probesEmission; // x: probesUsed, y: probeInfluence (Note: has to be interpreted as a float!), z: emissionStrength (Note: has to be interpreted as a float!)
	NEX_IVEC4 coneTracing; // x: cone tracing is used, y: coneTracingInfluence (Note: has to be interpreted as a float!)
#endif

	NEX_VEC4 diffuseSHCoefficients[9];
	NEX_IVEC4 specularReflectionIds;
	NEX_VEC4 specularReflectionWeights;

};

#define MAX_PER_OBJECT_MATERIAL_DATA 78

#ifndef __cplusplus //GLSL

#ifndef BUFFERS_DEFINE_OBJECT_BUFFER 
#define BUFFERS_DEFINE_OBJECT_BUFFER 0
#endif

#ifndef BUFFERS_DEFINE_MATERIAL_BUFFER 
#define BUFFERS_DEFINE_MATERIAL_BUFFER 0
#endif

	#if BUFFERS_DEFINE_OBJECT_BUFFER
		layout(column_major, std140, binding = OBJECT_SHADER_UNIFORM_BUFFER_BINDING_POINT) uniform PerObjectDataBuffer {
			PerObjectData objectData;
		};
	#endif
	
	#if BUFFERS_DEFINE_MATERIAL_BUFFER
		layout(column_major, std140, binding = SHADER_CONSTANTS_MATERIAL_BUFFER_BINDING_POINT) uniform MaterialBuffer {
			PerObjectMaterialData materials[MAX_PER_OBJECT_MATERIAL_DATA];
		};
	#endif

#endif



#ifdef __cplusplus
}
#endif 


#endif // SHADER_INTERFACE_BUFFERS_H