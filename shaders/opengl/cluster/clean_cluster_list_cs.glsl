#version 450 core

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;

layout (std430, binding = 0) buffer Input{
    uint clusterActive[];
};

layout (std430, binding = 1) buffer Output{
    uint globalActiveClusterCount;
    uint uniqueActiveClusters[];
};

//One compute shader for all clusters, one cluster per thread 
void main(void)
{
    uint clusterIndex  = gl_WorkGroupID.x +
                     gl_WorkGroupID.y * gl_NumWorkGroups.x +
                     gl_WorkGroupID.z * (gl_NumWorkGroups.x * gl_NumWorkGroups.y);
                     
    bool isActive = bool(clusterActive[clusterIndex]);
    
    if(isActive){
       uint offset = atomicAdd(globalActiveClusterCount, 1);
       uniqueActiveClusters[offset] = clusterIndex;
    }
}