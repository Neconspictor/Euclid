/**
 * Divides the camera frustum into clusters
 */
#version 430 core

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;

#include "interface/cluster_interface.h"


layout (std430, binding = 0) buffer ActiveClusterConstantssSSBO {
    ActiveClusterConstants constants;
};

layout (std430, binding = 1) buffer ActiveClustersSSBO {
    uint clusterActive[]; // bool is padded to 4 bytes; for clearity we use uint directly;
};

layout(binding = 0) uniform sampler2D depthTexture;


void main(void)
{
    //Getting the depth value
    vec2 texCoord = gl_WorkGroupID.xy / vec2(gl_NumWorkGroups.xy);
    float z = texture(depthTexture, texCoord).r; //reading the depth buffer
    //z = 0.1; //just validation now
    
    //Getting the linear cluster index value
    
    uint clusterZVal  = uint(log(z) * constants.numClusters.z / constants.zReproductionConstants.x - constants.zReproductionConstants.y);
    
    vec2 clusterPixelSize = gl_NumWorkGroups.xy / constants.numClusters.xy;

    uvec3 clusters    = uvec3( uvec2(gl_WorkGroupID.xy / clusterPixelSize), clusterZVal);
                               
    uint clusterID = clusters.x +
                        constants.numClusters.x * clusters.y +
                        (constants.numClusters.x * constants.numClusters.y) * clusters.z;
    
    clusterActive[clusterID] = uint(true);    
}