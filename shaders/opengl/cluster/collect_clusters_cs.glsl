/**
 * Divides the camera frustum into clusters
 */
#version 430 core

layout(local_size_x = 1, local_size_y = 1) in;



layout (std430, binding = 0) buffer ActiveClusters{
    //vec4 screenDimension;
    uvec4 numClusters; // cluster dimension in x,y and z axis; w component is unused
    vec4 constantsAB; // x: log(zFar / zNear), y: log(zNear) * numClusters.z / log(zFar/zNear)
    bool clusterActive[]; // bool has size of unsigned char
};

layout(binding = 0) uniform sampler2D depthTexture;

uint getClusterIndex(vec3 pixelCoord);
uint getDepthSlice(float z);

void main(void)
{
    //Getting the depth value
    vec2 screenCord = gl_WorkGroupID.xy / vec2(gl_NumWorkGroups.xy);
    float z = texture(depthTexture, screenCord).r; //reading the depth buffer
    z = 0.1; //just validation now
    
    //Getting the linear cluster index value
    //uint clusterID = getClusterIndex(vec3(pixelID.xy, z));
    
    uint clusterZVal  = uint(log(z) * numClusters.z / constantsAB.x - constantsAB.y);

    uvec3 clusters    = uvec3( gl_WorkGroupID.x, 
                               gl_WorkGroupID.y, 
                               clusterZVal);
                               
    uint clusterID = clusters.x +
                        numClusters.x * clusters.y +
                        (numClusters.x * numClusters.y) * clusters.z;
    
    clusterActive[clusterID] = true;
}

uint getClusterIndex(vec3 pixelCoord) {
    // Uses equation (3) from Building a Cluster Grid section
    uint clusterZVal  = getDepthSlice(pixelCoord.z);

    uvec3 clusters    = uvec3( pixelCoord.x / float(gl_NumWorkGroups.x), 
                               pixelCoord.y / float(gl_NumWorkGroups.y), 
                               clusterZVal);
                               
    uint clusterIndex = clusters.x +
                        numClusters.x * clusters.y +
                        (numClusters.x * numClusters.y) * clusters.z;
    return clusterIndex;
}

uint getDepthSlice(float z) {
    return uint(log(z) * numClusters.z / constantsAB.x - constantsAB.y);
}