/**
 * Maps a viewspace z value to the range [0,1]
 * @param z : viewspace z value to be mapped.
 * @param near: viewspace z value of the near plane
 * @param far: viewspace z value of the far plane
 */
float normalizeViewSpaceZ(float z, float near, float far) {    
    const float normalized = (z - near) / (far - near);
    return clamp(normalized, 0.0, 1.0);
}

/**
 * Denormalizes a normalized viewspace z value from the range [0,1] to [near, far].
 * Whereby near and far are the near resp. far plane viewspace z values.
 * The result will be the viewspace z value of the normalized z value.
 * @param normalizedZ: A normalized viewspace z value.
 * @param near: viewspace z value of the near plane
 * @param far: viewspace z value of the far plane
 */
float denormalizeViewSpaceZ(float normalizedZ, float near, float far) {
    return normalizedZ * (far - near) + near;
}

/**
 * Calculates a view space position from a view space z value, a texture coordinate and an inverse camera projection.
 */
vec3 getViewPositionFromNormalizedZ(in vec2 texCoord, in float viewSpaceZ, in mat4 inverseProjection) {
  
  const vec2 texNDC = texCoord * 2.0 - 1.0;
  const vec3 viewSpaceRay = (inverseProjection * vec4(texNDC, 1.0, 1.0)).xyz;
  // Note: we use a distance value. In opengl viewSpaceZ is negative, thus we use the negative 
  return -viewSpaceZ * viewSpaceRay;
};