/**
 * Calculates the minimum and maximum z-value of the scene (seen from the camera)  
 */
#version 430 core

#include "shadow/cascade_common.glsl"

#define FLT_MAX 3.402823466e+38

struct GlobalShadow
{
    mat4 worldToShadowSpace;
    mat4 shadowView;
    float radius;
};

struct BoundingSphere
{
    vec3 center;
    float radius;
};

struct Frustum {
    vec3 corners[8];
};

/**
 * A buffer that holds writable cascade data, that is needed by other shaders, too.
 */
layout(std140, binding = 0) buffer writeonly SharedOutput
{
    CascadeData data;
    //mat4 matrix;
} sharedOutput;


/**
 * A buffer that holds writable data, that is only needed by this shader
 */
layout(std430, binding = 1) buffer PrivateOutput
{
    vec4 cascadeBoundCenters[CSM_NUM_CASCADES]; // w component has to be 1.0!
} privateOutput;


/**
 * A buffer that holds readonly input
 */
layout(std430, binding = 2) buffer readonly ConstantInput
{
   mat4 viewMatrix;
   mat4 projectionMatrix;
   vec4 lightDirection; // w-component isn't used
   vec4 nearFarPlane; // x and y component hold the near and far plane of the camera
   vec4 shadowMapSize; // only x component is used
   vec4 cameraPostionWS; // w component isn't used
   vec4 cameraLook; // w component isn't used
} constantInput;

/**
 * A buffer that holds readonly input
 */
layout(std430, binding = 3) buffer DistanceInput
{
   vec4 minMax; // x and y component hold the min and max (positive) viewspace z value; the other components are not used
   
} distanceInput;




uniform uint useAntiFlickering;


/**
 * Calcs a orthogonal projection matrix for a right-handed coordinate system and a z-range [-1, 1] in NDC space
 */
mat4 ortho(float left, float right, float bottom, float top, float zNear, float zFar)
{
    mat4 result;
    // Note: glsl uses columns as the first index and rows for the second!
    result[0][0] = 2.0 / (right - left);
    result[1][1] = 2.0 / (top - bottom);
    result[2][2] = -2.0 / (zFar - zNear);
    
    result[3][0] = -(right + left) / (right - left);
    result[3][1] = -(top + bottom) / (top - bottom);
    result[3][2] = -(zFar + zNear) / (zFar - zNear);
    
    result[3][3] = 1.0;
    return result;
}


/**
 * Calcs a right-handed look-at matrix
 */
mat4 lookAt(in vec3 eye, in vec3 center, in vec3 up) {
    const vec3 f = normalize(center - eye);
    const vec3 s = normalize(cross(f, up));
    const vec3 u = cross(s, f);

    mat4 result;
    // Note: glsl uses columns as the first index and rows for the second!
    result[0][0] = s.x;
    result[1][0] = s.y;
    result[2][0] = s.z;
    
    result[0][1] = u.x;
    result[1][1] = u.y;
    result[2][1] = u.z;
    
    result[0][2] =-f.x;
    result[1][2] =-f.y;
    result[2][2] =-f.z;
    
    result[3][0] =-dot(s, eye);
    result[3][1] =-dot(u, eye);
    result[3][2] = dot(f, eye);
    
    result[3][3] = 1.0;
    return result;
}


void calcSplitSchemes(in vec2 minMaxPositiveZ) {

    const float range = minMaxPositiveZ.y - minMaxPositiveZ.x;
    const float step = range / float(CSM_NUM_CASCADES);
    float splitDistances[CSM_NUM_CASCADES];
   
   
   for (uint i = 0; i < CSM_NUM_CASCADES; ++i) {
       splitDistances[i] = ((i + 1)*step);
       splitDistances[i] = ceil(splitDistances[i] * 32.0f) / 32.0f;
   }
   
   const float nearClip = minMaxPositiveZ.x;

	// We calculate the splitting planes of the view frustum by using an algorithm 
	for (uint i = 0; i < CSM_NUM_CASCADES; ++i)
	{
		sharedOutput.data.cascadedSplits[i].x = (nearClip + splitDistances[i]);
	}
}


Frustum extractFrustumPoints(in float nearSplitDistance, in float farSplitDistance) {

    Frustum frustum;
    
    // At first we define the frustum corners in NDC space
	frustum.corners = vec3[8]
	(
		vec3(-1.0,  1.0, -1.0),
		vec3(1.0,  1.0, -1.0),
		vec3(1.0, -1.0, -1.0),
		vec3(-1.0, -1.0, -1.0),
		vec3(-1.0,  1.0,  1.0),
		vec3(1.0,  1.0,  1.0),
		vec3(1.0, -1.0,  1.0),
		vec3(-1.0, -1.0,  1.0)
	);
    
    // Now we transform the frustum corners back to world space
	mat4 invViewProj = inverse(constantInput.projectionMatrix * constantInput.viewMatrix);
	for (uint i = 0; i < 8; ++i)
	{
		vec4 inversePoint = invViewProj * vec4(frustum.corners[i], 1.0f);
		frustum.corners[i] = vec3(inversePoint / inversePoint.w);
	}
    
    const float clipRange = constantInput.nearFarPlane.y - constantInput.nearFarPlane.x;
    
    // Calculate rays that define the near and far plane of each cascade split.
	// Than we translate the frustum corner accordingly so that the frustum starts at the near splitting plane
	// and ends at the far splitting plane.
	for (uint i = 0; i < 4; ++i)
	{
		vec3 cornerRay = frustum.corners[i + 4] - frustum.corners[i];
		vec3 nearCornerRay = cornerRay * nearSplitDistance / clipRange;
		vec3 farCornerRay = cornerRay * farSplitDistance / clipRange;
		frustum.corners[i + 4] = frustum.corners[i] + farCornerRay;
		frustum.corners[i] = frustum.corners[i] + nearCornerRay;
	}
    
    return frustum;
}


BoundingSphere extractFrustumBoundSphere(in float nearSplitDistance, in float farSplitDistance) {

    Frustum frustumWS = extractFrustumPoints(nearSplitDistance, farSplitDistance);
    
    // calc center of the frustum
	vec3 frustumCenter = vec3(0.0);
	for (uint i = 0; i < 8; ++i) {
        frustumCenter += frustumWS.corners[i];
    }
	frustumCenter /= 8.0;
    
    // calc sphere that tightly encloses the frustum
	// We use the max distance from the frustum center to the corners
	// TODO Actually should the distance ot all corners be the same???
	float radius = 0.0;
	for (uint i = 0; i < 8; ++i)
	{
		float distance = length(frustumWS.corners[i] - frustumCenter);
		radius = max(radius, distance);
	}
    
    // do some rounding for fighting numerical issues
	// This helps to reduce flickering
	// Note that we use here a different formula than in function calcSplitDistances, as it produces better results 
	radius = round(radius * 16.0) / 16.0;
    
    BoundingSphere result;
    result.center = frustumCenter;
    result.radius = radius;
    
    return result;
}


GlobalShadow calcShadowSpace(in float nearPlane, in float farPlane, in vec3 lightDirection) {

    const float cascadeTotalRange = farPlane - nearPlane;

    BoundingSphere shadowBounds = extractFrustumBoundSphere(nearPlane, farPlane);
    
    // Find the (global)shadow view matrix
	const vec3 cameraFrustumCenterWS = constantInput.cameraPostionWS.xyz + constantInput.cameraLook.xyz * cascadeTotalRange * 0.5;
    const vec3 lightPos = cameraFrustumCenterWS + normalize(lightDirection) * shadowBounds.radius;
    
    
    vec3 upVec = normalize(cross(lightDirection, vec3(0.0f, 1.0f, 0.0f)));
	if (length(upVec) < 0.999)
	{
		upVec = normalize(cross(lightDirection, vec3(1.0f, 0.0f, 0.0f)));
	}

	const mat4 shadowView = lookAt(cameraFrustumCenterWS, lightPos, upVec);
    
    
    // Get the bounds for the shadow space

	const mat4 shadowProj = ortho(-shadowBounds.radius, shadowBounds.radius,
		-shadowBounds.radius, shadowBounds.radius,
		-shadowBounds.radius, shadowBounds.radius);
    
    // The combined transformation from world to shadow space
	GlobalShadow result;
	result.worldToShadowSpace = shadowProj * shadowView;
	result.shadowView = shadowView;
	result.radius = shadowBounds.radius;
	return result;
}

bool cascadeNeedsUpdate(in mat4 shadowView, in uint cascadeIdx, in vec3 newCenter, in vec3 oldCenter, 
                        in float cascadeBoundRadius, out vec3 offset)
{
	// Find the offset between the new and old bound center
	const vec3 oldCenterInCascade = vec3(shadowView * vec4(oldCenter, 1.0f));
	const vec3 newCenterInCascade = vec3(shadowView * vec4(newCenter, 1.0f));
	const vec3 centerDiff = newCenterInCascade - oldCenterInCascade;

	// Find the pixel size based on the diameters and map pixel size
	const float pixelSize = float(constantInput.shadowMapSize.x) / (2.0 * cascadeBoundRadius);

	const float pixelOffX = centerDiff.x * pixelSize;
	const float pixelOffY = centerDiff.y * pixelSize;
	//const float pixelOffZ = centerDiff.z * pixelSize;

	// Check if the center moved at least half a pixel unit
	const bool needUpdate = abs(pixelOffX) > 0.5 || abs(pixelOffY) > 0.5; //|| abs(pixelOffZ) > 0.5f;
	if (needUpdate)
	{
		// Round to the 
		offset.x = floor(0.5 + pixelOffX) / pixelSize;
		offset.y = floor(0.5 + pixelOffY) / pixelSize;
		//offset.z = floor(0.5 + pixelOffZ) / pixelSize;
		offset.z = centerDiff.z;
	}

	return needUpdate;
}


layout (local_size_x = 1, local_size_y = 1, local_size_z = 1) in;

void main(void)
{
    //sharedOutput.data.inverseViewMatrix = mat4();
    //sharedOutput.data.inverseViewMatrix[0] = vec4(1,2,3,4);
    
    
    /*uint id = gl_GlobalInvocationID.x + gl_GlobalInvocationID.y + gl_GlobalInvocationID.z; 
    
    if (id == 0) {
        if (sharedOutput.matrix[0][0] == 1.0) {
            sharedOutput.matrix = mat4(5,6,7,8,5,6,7,8,5,6,7,8,5,6,7,8);
        } else {
            sharedOutput.matrix = mat4(1,2,3,4,1,2,3,4,1,2,3,4,1,2,3,4);
        }
    }*/
    
    calcSplitSchemes(distanceInput.minMax.xy);
    sharedOutput.data.inverseViewMatrix = inverse(constantInput.viewMatrix);
    
    GlobalShadow global = calcShadowSpace(constantInput.nearFarPlane.x, constantInput.nearFarPlane.y, constantInput.lightDirection.xyz);
    const mat3 shadowOffsetMatrix = mat3(transpose(global.shadowView));
    const float minDistance = distanceInput.minMax.x;
    
    for (uint cascadeIdx = 0; cascadeIdx < CSM_NUM_CASCADES; ++cascadeIdx)
	{
		const float nearSplitDistance = (cascadeIdx == 0 ? minDistance : sharedOutput.data.cascadedSplits[cascadeIdx - 1].x);
		const float farSplitDistance = sharedOutput.data.cascadedSplits[cascadeIdx].x;


		mat4 cascadeTrans;
		mat4 cascadeScale;
		vec3 cascadeCenterShadowSpace;
		float scale;

        // We don't care about branching as this compute shader is only run on one thread instance!
		if (useAntiFlickering)
		{

			// To avoid anti flickering we need to make the transformation invariant to camera rotation and translation
			// By encapsulating the cascade frustum with a sphere we achive the rotation invariance
			const BoundingSphere boundingSphere = extractFrustumBoundSphere(nearSplitDistance, farSplitDistance);
			const float radius = boundingSphere.radius;
			const vec3 frustumCenterWS = boundingSphere.center;

			// Only update the cascade bounds if it moved at least a full pixel unit
			// This makes the transformation invariant to translation
			vec3 offset;
			if (cascadeNeedsUpdate(global.shadowView, cascadeIdx, frustumCenterWS, privateOutput.cascadeBoundCenters[cascadeIdx].xyz, radius, offset))
			{
				// To avoid flickering we need to move the bound center in full units
				// NOTE: we don't want translation affect the offset!
				vec3 offsetWS = shadowOffsetMatrix * offset;
				privateOutput.cascadeBoundCenters[cascadeIdx] += vec4(offsetWS, 0.0);
			}

			// Get the cascade center in shadow space
			cascadeCenterShadowSpace = vec3(global.worldToShadowSpace * vec4(privateOutput.cascadeBoundCenters[cascadeIdx].xyz, 1.0));

			// Update the scale from shadow to cascade space
			scale = global.radius / radius;
		}
		else
		{
            Frustum frustumWS = extractFrustumPoints(nearSplitDistance, farSplitDistance);
			vec3 maxExtents = vec3(-FLT_MAX, -FLT_MAX, -FLT_MAX);
			vec3 minExtents = vec3(FLT_MAX, FLT_MAX, FLT_MAX);

			for (uint i = 0; i < 8; ++i)
			{
				vec3 pointInShadowSpace = vec3(global.worldToShadowSpace * vec4(frustumWS.corners[i], 1.0f));
				minExtents = min(minExtents, pointInShadowSpace);
				maxExtents = max(maxExtents, pointInShadowSpace);
			}

			cascadeCenterShadowSpace = 0.5 * (minExtents + maxExtents);
			scale = 2.0 / max(maxExtents.x - minExtents.x, maxExtents.y - minExtents.y);
		}


		// Update the translation from shadow to cascade space  
        // translate by vec3(-cascadeCenterShadowSpace.x, -cascadeCenterShadowSpace.y, 0.0)
        //cascadeTrans = mat4(1.0, 0.0, 0.0, 0.0,  0.0, 1.0, 0.0, 0.0,  0.0, 0.0, 1.0, 0.0,  
        //                    -cascadeCenterShadowSpace.x, -cascadeCenterShadowSpace.y, 0.0, 1.0);
                            
        cascadeTrans = mat4(1.0, 0.0, 0.0, 0.0,  
                            0.0, 1.0, 0.0, 0.0,  
                            0.0, 0.0, 1.0, 0.0,  
                            -cascadeCenterShadowSpace.x, -cascadeCenterShadowSpace.y, 0.0, 1.0);

        // scale by vec3(scale, scale, 1.0)
        cascadeScale = mat4(scale, 0.0, 0.0, 0.0,  
                            0.0, scale, 0.0, 0.0,  
                            0.0, 0.0, 1.0, 0.0,  
                            0.0, 0.0, 0.0, 1.0);

		//Store the split distances and the relevant matrices
		sharedOutput.data.lightViewProjectionMatrices[cascadeIdx] = cascadeScale * cascadeTrans * global.worldToShadowSpace;
        
        //sharedOutput.data.lightViewProjectionMatrices[cascadeIdx] = cascadeTrans;
        
		sharedOutput.data.scaleFactors[cascadeIdx].x = scale;
        //sharedOutput.minMax = distanceInput.minMax;
	}
}