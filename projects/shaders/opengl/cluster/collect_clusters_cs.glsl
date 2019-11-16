/**
 * Divides the camera frustum into clusters
 */
#version 430 core

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;

#include "interface/cluster_interface.h"


layout (std140, binding = 0) uniform ActiveClusterConstantssSSBO {
    ActiveClusterConstants constants;
};

layout (std430, binding = 0) buffer ActiveClustersSSBO {
    float clusterActive[]; // bool is padded to 4 bytes; for clearity we use uint directly;
};

layout(binding = 0) uniform sampler2D depthTexture;

float getZDistance(float depth);

void main(void)
{
    //Getting the depth value
    vec2 texCoord = vec2(float(gl_WorkGroupID.x) / float(gl_NumWorkGroups.x), float(gl_WorkGroupID.y) / float(gl_NumWorkGroups.y));
    float depth = clamp(texture(depthTexture, texCoord).r, 0.0, 1.0); //reading the depth buffer
    float dist = getZDistance(depth);
    float logZ = log(dist); 
    //z = 0.1; //just validation now
    
    //Getting the linear cluster index value
    
    float clusterZVal  = floor((logZ * constants.numClusters.z / constants.zReproductionConstants.x) - constants.zReproductionConstants.y);
    uint clusterZ = uint(clusterZVal);
    
    if (clusterZ > (constants.numClusters.z - 1)) {
        return;
    }
    
    vec2 clusterPixelSize = vec2(gl_NumWorkGroups.xy) / constants.numClusters.xy;



    uvec3 clusters    = uvec3( uvec2(vec2(gl_WorkGroupID.xy) / clusterPixelSize), clusterZ);
                               
    uint clusterID = clusters.x +
                        uint(constants.numClusters.x) * clusters.y +
                        (uint(constants.numClusters.x) * uint(constants.numClusters.y)) * clusters.z;
    
    //atomicExchange(clusterActive[clusterID], 1);
    
    //if (dist < 0.2) {
    
    //clusterActive[gl_WorkGroupID.x + gl_WorkGroupID.y * gl_NumWorkGroups.x] = clusterID;//float(clusterZ);
    clusterActive[clusterID] = 1;
        
    //}
    
    
    //if (clusterID == 1) {
    //    clusterActive[1] = 1;
    //}
}


float getZDistance(float depth) {
    /*
        A     = prj_mat[2][2];
        B     = prj_mat[3][2];
        z_ndc = 2.0 * depth - 1.0;
        z_eye = B / (A + z_ndc);
    */
    
    const float A = constants.proj[2][2];
    const float B = constants.proj[3][2];
    const float z_ndc = 2.0 * depth - 1.0;
    const float z_eye = -B / (A + z_ndc);
    
    return -z_eye;
}