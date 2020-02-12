/**
  Necessary inputs: 
  
 layout(std140, binding = VOXEL_C_UNIFORM_BUFFER_BINDING_POINT) uniform Cbuffer {
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

layout(binding = VOXEL_TEXTURE_BINDING_POINT) uniform sampler3D voxelTexture;

 
 */

#ifndef VOXEL_CONE_TRACE_HEADER
#define VOXEL_CONE_TRACE_HEADER

#include "GI/util.glsl"

const vec3 CONES[] = 
{
	vec3(0.57735, 0.57735, 0.57735),
	vec3(0.57735, -0.57735, -0.57735),
	vec3(-0.57735, 0.57735, -0.57735),
	vec3(-0.57735, -0.57735, 0.57735),
	vec3(-0.903007, -0.182696, -0.388844),
	vec3(-0.903007, 0.182696, 0.388844),
	vec3(0.903007, -0.182696, 0.388844),
	vec3(0.903007, 0.182696, -0.388844),
	vec3(-0.388844, -0.903007, -0.182696),
	vec3(0.388844, -0.903007, 0.182696),
	vec3(0.388844, 0.903007, -0.182696),
	vec3(-0.388844, 0.903007, 0.182696),
	vec3(-0.182696, -0.388844, -0.903007),
	vec3(0.182696, 0.388844, -0.903007),
	vec3(-0.182696, 0.388844, 0.903007),
	vec3(0.182696, -0.388844, 0.903007)
};

const float MAX_DIST = 100;
const float	SQRT2 = 1.41421356237309504880;


// voxelTexture:	3D Texture containing voxel scene with direct diffuse lighting (or direct + secondary indirect bounce)
// P:				world-space position of receiving surface
// N:				world-space normal vector of receiving surface
// coneDirection:	world-space cone direction in the direction to perform the trace
// coneAperture:	tan(coneHalfAngle)
vec4 ConeTrace(in vec3 P, in vec3 N, in vec3 coneDirection, in float coneAperture)
{

	vec3 color = vec3(0.0);
	float alpha = 0.0;
	
	// We need to offset the cone start position to avoid sampling its own voxel (self-occlusion):
	//	Unfortunately, it will result in disconnection between nearby surfaces :(
	float dist = 0.0;//g_xFrame_VoxelRadianceDataSize; // offset by cone dir so that first sample of all cones are not the same
	
    vec3 startPos = P + N * constants.voxels.g_xFrame_VoxelRadianceDataSize * 2 * SQRT2; // sqrt2 is diagonal voxel half-extent
    
    /*vec3 diff = startPos * g_xFrame_VoxelRadianceDataRes_rcp * g_xFrame_VoxelRadianceDataSize_rcp;
    vec3 uvw = diff * 0.5 + 0.5;
    
    if (!is_saturated(uvw)) return vec4(0.0);
    
    vec4 col =  textureLod(voxelTexture, uvw, 0);
    float a = 1 - col.a;
		color += a * col.rgb;
		alpha += a * col.a;
        
    return vec4(color, alpha);*/
    

	// We will break off the loop if the sampling distance is too far for performance reasons:
	const float maxDistance = MAX_DIST * constants.voxels.g_xFrame_VoxelRadianceDataSize;

	while (dist < maxDistance && alpha < 1)
	{
		float diameter = max(constants.voxels.g_xFrame_VoxelRadianceDataSize, 2 * coneAperture * dist);
		float mip = log2(diameter * constants.voxels.g_xFrame_VoxelRadianceDataSize_rcp);

		// Because we do the ray-marching in world space, we need to remap into 3d texture space before sampling:
		//	todo: optimization could be doing ray-marching in texture space
		vec3 tc = startPos + coneDirection * dist;
		tc = (tc) * constants.voxels.g_xFrame_VoxelRadianceDataSize_rcp; //tc - g_xFrame_VoxelRadianceDataCenter.xyz
		tc *= constants.voxels.g_xFrame_VoxelRadianceDataRes_rcp;
		tc = tc * 0.5 + 0.5;

		// break if the ray exits the voxel grid, or we sample from the last mip:
		if (!is_saturated(tc) || mip >= float(constants.voxels.g_xFrame_VoxelRadianceDataMIPs))
			break;

		vec4 sam = textureLod(voxelTexture, tc, mip);
        

		// this is the correct blending to avoid black-staircase artifact (ray stepped front-to back, so blend front to back):
		float a = 1 - alpha;
		color += mix(a,sam.a, 0.5) * sam.rgb; /// (max(dist * 0.1, 1.0)) a * sam.rgb
		alpha += a * sam.a; //	alpha += a * sam.a;

		// step along ray:
		dist += diameter * constants.voxels.g_xFrame_VoxelRadianceRayStepSize;
	}

	return vec4(color, 1.0);
}

// P:				world-space position of receiving surface
// N:				world-space normal vector of receiving surface
vec4 ConeTraceRadiance(in vec3 P, in vec3 N)
{

    //return vec4(0.0,0.0,0.0,1.0);

	vec4 radiance = vec4(0.0);
    const float aperture = tan(PI * 0.5 * 0.33);

	for (uint cone = 0; cone < constants.voxels.g_xFrame_VoxelRadianceNumCones; ++cone) // quality is between 1 and 16 cones
	{
		// approximate a hemisphere from random points inside a sphere:
		//  (and modulate cone with surface normal, no banding this way)
		vec3 coneDirection = normalize(CONES[cone] + N);
		// if point on sphere is facing below normal (so it's located on bottom hemisphere), put it on the opposite hemisphere instead:
		coneDirection *= dot(N, coneDirection) < 0 ? -1 : 1;

		radiance += ConeTrace(P, N, coneDirection, aperture);
	}

	// final radiance is average of all the cones radiances
	radiance *= constants.voxels.g_xFrame_VoxelRadianceNumCones_rcp;
	radiance.a = clamp(radiance.a, 0.0, 1.0);

	return max(vec4(0.0), radiance);
}

// P:				world-space position of receiving surface
// N:				world-space normal vector of receiving surface
// V:				world-space view-vector (cameraPosition - P)
vec4 ConeTraceReflection(in vec3 P, in vec3 N, in vec3 V, in float roughness)
{
	const float aperture = tan(roughness * PI * 0.5 * 0.1);
	vec3 coneDirection = reflect(-V, N);

	vec4 reflection = ConeTrace(P, N, coneDirection, aperture);

	return vec4(max(vec3(0.0), reflection.rgb), clamp(reflection.a, 0.0, 1.0));
}

#endif
