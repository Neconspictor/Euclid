#ifndef DEPTH_UTIL_H
#define DEPTH_UTIL_H

#ifndef NDC_Z_ZERO_TO_ONE
	#error("NDC_Z_ZERO_TO_ONE has to be defined")
#endif




//NOTE: NDC_Z_ZERO_TO_ONE defines whether the z ndc range is [0,1]. If not defined 
// the opengl default range ([-1,1]) is used.

/**
 * Transforms a depth value from screen space to ndc space
 */
float screenToNDC(in float depth) {
  #if NDC_Z_ZERO_TO_ONE
	return depth;
  #else 
	return depth * 2.0f - 1.0f;
  #endif
};

/**
 * Transforms a ndc z value to screen space.
 */
float z_ndcToScreen(in float z) {
  #if NDC_Z_ZERO_TO_ONE
	return z;
  #else 
	return z * 0.5f + 0.5f;
  #endif
};



/**
 * Computes a position from a depth value in screen space to a target space specified by an inverse transformation matrix.
 *
 * @param invTrans : The inverse transformation matrix to the target space. 
 *                   E.g. for coputing world space position use the inverse view-projection matrix. 
 *                   Analogously for getting the position in view space use the inverse projection matrix.
 * @param texCoord : The texture coordinates (screen space range [0,1]) of a pixel used for retrieving the depth value.
 * @param depth : The depth value.
 */
vec3 reconstructPositionFromDepth(in mat4 invTrans, in vec2 texCoord, in float depth) {
  
  vec4 ndcLocation;
  ndcLocation.xy = texCoord * 2.0f - 1.0f;
  ndcLocation.z = screenToNDC(depth);
  ndcLocation.w = 1.0f;
  vec4 position = invTrans * ndcLocation;
  
  // Finally reverse perspective division
  return position.xyz / position.w;
};

#endif //DEPTH_UTIL_H