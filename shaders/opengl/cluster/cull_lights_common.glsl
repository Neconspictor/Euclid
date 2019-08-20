/**
 * Requirements: A shared local array of light objects having a 'position' field (world space)
 * The array has to contain LOCAL_SIZE_X * LOCAL_SIZE_Y * LOCAL_SIZE_Z elements.
 * An array of type LIGHT_CLASS is needed.
 * The macros:
 * LOCAL_SIZE_X : local workgroup size (x dimension)
 * LOCAL_SIZE_Y : local workgroup size (Y dimension)
 * LOCAL_SIZE_Z : local workgroup size (Z dimension)
 * MAX_VISIBLES_LIGHTS: the maximum of lights able to affect a cluster
 * LIGHT_CLASS : Specify the concrete light struct. 
 * SUPPORTS_SPHERE_RANGE : Specifies that the LIGHT_CLASS struct contains a 'sphereRange' field (float).
 * SUPPORTS_AABB : Specifies that the LIGHT_CLASS struct contains a 'minWorld' and 'maxWorld' field (both vec4).
 */

#ifndef MAX_VISIBLES_LIGHTS
    #define MAX_VISIBLES_LIGHTS 100
#endif
#ifndef LOCAL_SIZE_X
    #define LOCAL_SIZE_X 16
#endif
#ifndef LOCAL_SIZE_Y
    #define LOCAL_SIZE_Y 8
#endif
#ifndef LOCAL_SIZE_Z
    #define LOCAL_SIZE_Z 4
#endif


#include "interface/cluster_interface.h"
//#include "interface/light_interface.h"


layout (std140, binding = 0) uniform ConstantsUBO 
{
    Constants constants;
};

layout (std430, binding = 0) buffer clusterAABB 
{
    AABB clusters[];
};


/**
 * Must have size of MAX_VISIBLES_LIGHTS * clusterCount
 */
layout (std430, binding = 1) buffer lightIndexSSBO 
{
    uint globalLightIndexList[];
};

/**
 * Stores offset and count into globalLightIndexList for each cluster.
 * => Size of array: amount of clusters
 */
layout (std430, binding = 2) buffer lightGridSSBO 
{
    LightGrid lightGrids[];
};

layout (std430, binding = 3) buffer globalIndexCountSSBO 
{
    uint globalIndexCount;
};

//Shared variables 
shared LIGHT_CLASS sharedLights[LOCAL_SIZE_X * LOCAL_SIZE_Y * LOCAL_SIZE_Z];


float sqDistPointAABB(vec3 point, uint clusterID)
{
    float sqDist = 0.0;
    AABB currentCell = clusters[clusterID];
	
	
	const vec4 minView = currentCell.minView;
	const vec4 maxView = currentCell.maxView;

	if(point[0] < minView[0])
	{
		float dist = minView[0] - point[0];
		sqDist += dist * dist;
	}
	
	if(point[1] < minView[1])
	{
		float dist = minView[1] - point[1];
		sqDist += dist * dist;
	}
	
	if(point[2] < minView[2])
	{
		float dist = minView[2] - point[2];
		sqDist += dist * dist;
	}
	
	if(point[0] > maxView[0])
	{
		float dist = point[0] - maxView[0];
		sqDist += dist * dist;
	}
	
	if(point[1] > maxView[1])
	{
		float dist = point[1] - maxView[1];
		sqDist += dist * dist;
	}
	
	if(point[2] > maxView[2])
	{
		float dist = point[2] - maxView[2];
		sqDist += dist * dist;
	}

    return sqDist;
}

#ifdef SUPPORTS_AABB

bool testAABBWorld(uint lightID, uint clusterID) {
    AABB currentCell = clusters[clusterID];
    LIGHT_CLASS light = lights[lightID];
    
    return (currentCell.minWorld.x <= light.maxWorld.x && currentCell.maxWorld.x >= light.minWorld.x) &&
    (currentCell.minWorld.y <= light.maxWorld.y && currentCell.maxWorld.y >= light.minWorld.y) &&
    (currentCell.minWorld.z <= light.maxWorld.z && currentCell.maxWorld.z >= light.minWorld.z);
}

#endif //SUPPORTS_AABB

#ifdef SUPPORTS_SPHERE_RANGE

bool testSphereAABB(uint light, uint clusterID)
{
    float radius = sharedLights[light].sphereRange;
    vec3 center  = vec3(constants.view * sharedLights[light].position);
    float squaredDistance = sqDistPointAABB(center, clusterID);

    return squaredDistance <= (radius * radius);
}

#endif //SUPPORTS_SPHERE_RANGE