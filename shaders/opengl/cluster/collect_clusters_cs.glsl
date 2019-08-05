/**
 * Divides the camera frustum into clusters
 */
#version 430 core

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;



layout (std430, binding = 0) buffer ActiveClusters{
    //vec4 screenDimension;
    uvec4 numClusters; // cluster dimension in x,y and z axis; w component is unused
    vec4 constantsAB; // x: log(zFarDistance / zNearDistance), y: log(zNearDistance) * numClusters.z / log(zFarDistance/zNearDistance)
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
    
    uint clusterZVal  = uint(log(z) * numClusters.z / constantsAB.x - constantsAB.y);
    
    vec2 clusterPixelSize = gl_NumWorkGroups.xy / numClusters.xy;

    uvec3 clusters    = uvec3( uvec2(gl_WorkGroupID.xy / clusterPixelSize), clusterZVal);
                               
    uint clusterID = clusters.x +
                        numClusters.x * clusters.y +
                        (numClusters.x * numClusters.y) * clusters.z;
    
    clusterActive[clusterID] = uint(true);
    
    /*for (uint i = 0; i < numClusters.x; ++i) {
        for (uint j = 0; j < numClusters.y; ++j) {
            for (uint k = 0; k < numClusters.z; ++k) {
                uint id = i + 
                            numClusters.x * j + 
                            (numClusters.x * numClusters.y) * k;
                clusterActive[id] = uint(true);            
            }
        }
    }*/
    
}