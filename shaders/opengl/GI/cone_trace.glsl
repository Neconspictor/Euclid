#ifndef VOXEL_CONE_TRACE_HEADER
#define VOXEL_CONE_TRACE_HEADER

#ifndef VOXEL_C_UNIFORM_BUFFER_BINDING_POINT
#define VOXEL_C_UNIFORM_BUFFER_BINDING_POINT 0
#endif

#ifndef VOXEL_TEXTURE_BINDING_POINT
#define VOXEL_TEXTURE_BINDING_POINT 0
#endif


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

#include "GI/util.glsl"

layout(binding = VOXEL_TEXTURE_BINDING_POINT) uniform sampler3D voxelTexture;

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
vec4 ConeTrace(in vec3 P, in float3 N, in vec3 coneDirection, in float coneAperture)
{
	vec3 color = 0;
	float alpha = 0;
	
	// We need to offset the cone start position to avoid sampling its own voxel (self-occlusion):
	//	Unfortunately, it will result in disconnection between nearby surfaces :(
	float dist = g_xFrame_VoxelRadianceDataSize; // offset by cone dir so that first sample of all cones are not the same
	vec3 startPos = P + N * g_xFrame_VoxelRadianceDataSize * 2 * SQRT2; // sqrt2 is diagonal voxel half-extent

	// We will break off the loop if the sampling distance is too far for performance reasons:
	const float maxDistance = MAX_DIST * g_xFrame_VoxelRadianceDataSize;

	while (dist < maxDistance && alpha < 1)
	{
		float diameter = max(g_xFrame_VoxelRadianceDataSize, 2 * coneAperture * dist);
		float mip = log2(diameter * g_xFrame_VoxelRadianceDataSize_rcp);

		// Because we do the ray-marching in world space, we need to remap into 3d texture space before sampling:
		//	todo: optimization could be doing ray-marching in texture space
		vec3 tc = startPos + coneDirection * dist;
		tc = (tc - g_xFrame_VoxelRadianceDataCenter) * g_xFrame_VoxelRadianceDataSize_rcp;
		tc *= g_xFrame_VoxelRadianceDataRes_rcp;
		tc = tc * 0.5 + 0.5;

		// break if the ray exits the voxel grid, or we sample from the last mip:
		if (!is_saturated(tc) || mip >= (float)g_xFrame_VoxelRadianceDataMIPs)
			break;

		vec4 sam = textureLod(voxelTexture, tc, mip);

		// this is the correct blending to avoid black-staircase artifact (ray stepped front-to back, so blend front to back):
		float a = 1 - alpha;
		color += a * sam.rgb;
		alpha += a * sam.a;

		// step along ray:
		dist += diameter * g_xFrame_VoxelRadianceRayStepSize;
	}

	return vec4(color, alpha);
}

// P:				world-space position of receiving surface
// N:				world-space normal vector of receiving surface
vec4 ConeTraceRadiance(in vec3 P, in vec3 N)
{
	vec4 radiance = 0;
    const float aperture = tan(PI * 0.5 * 0.33);

	for (uint cone = 0; cone < g_xFrame_VoxelRadianceNumCones; ++cone) // quality is between 1 and 16 cones
	{
		// approximate a hemisphere from random points inside a sphere:
		//  (and modulate cone with surface normal, no banding this way)
		vec3 coneDirection = normalize(CONES[cone] + N);
		// if point on sphere is facing below normal (so it's located on bottom hemisphere), put it on the opposite hemisphere instead:
		coneDirection *= dot(coneDirection, N) < 0 ? -1 : 1;

		radiance += ConeTrace(P, N, coneDirection, aperture);
	}

	// final radiance is average of all the cones radiances
	radiance *= g_xFrame_VoxelRadianceNumCones_rcp;
	radiance.a = clamp(radiance.a, 0.0, 1.0);

	return max(0, radiance);
}

// P:				world-space position of receiving surface
// N:				world-space normal vector of receiving surface
// V:				world-space view-vector (cameraPosition - P)
vec4 ConeTraceReflection(in vec3 P, in vec3 N, in vec3 V, in float roughness)
{
	const float aperture = tan(roughness * PI * 0.5 * 0.1);
	vec3 coneDirection = reflect(-V, N);

	vec4 reflection = ConeTrace(P, N, coneDirection, aperture);

	return float4(max(0, reflection.rgb), clamp(reflection.a, 0.0, 1.0));
}

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

#endif
