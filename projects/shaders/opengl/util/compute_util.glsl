/**
 * Provides a flattend version of gl_GlobalInvocationID
 */
uint getGlobalInvocationIndex() {
    uvec3 globalSize = gl_NumWorkGroups * gl_WorkGroupSize;
    return gl_GlobalInvocationID.z * globalSize.x * globalSize.y + 
                                 gl_GlobalInvocationID.y * globalSize.x + 
                                 gl_GlobalInvocationID.x;
}