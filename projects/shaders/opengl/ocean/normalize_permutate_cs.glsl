#version 460 core

#define GROUP_NUM_X 1
#define GROUP_NUM_Y 1

layout (local_size_x = GROUP_NUM_X, local_size_y = GROUP_NUM_Y) in;

// read-/write
layout (rg32f, binding = 0) uniform image2D height;
layout (rg32f, binding = 1) uniform image2D slopeX;
layout (rg32f, binding = 2) uniform image2D slopeZ;
layout (rg32f, binding = 3) uniform image2D dX;
layout (rg32f, binding = 4) uniform image2D dZ;

uniform int N;

void main(void)
{
    const ivec2 index = ivec2(gl_GlobalInvocationID.xy);
    
    // height
    float signs[] = { 1.0, -1.0};
    float permutation = signs[(index.x + index.y) & 1];
    vec2 h = permutation * imageLoad(height, index).xy / float(N);
    imageStore(height, index, vec4(h.x, h.y, 0.0, 0.0));
    
    // slopeX
    vec2 sX = permutation * imageLoad(slopeX, index).xy / float(N);
    imageStore(slopeX, index, vec4(sX.x, sX.y, 0.0, 0.0));
    
    // slopeZ
    vec2 sZ = permutation * imageLoad(slopeZ, index).xy / float(N);
    imageStore(slopeZ, index, vec4(sZ.x, sZ.y, 0.0, 0.0));
    
    // dX
    vec2 dxSample = permutation * imageLoad(dX, index).xy / float(N);
    imageStore(dX, index, vec4(dxSample.x, dxSample.y, 0.0, 0.0));
    
    // dZ
    vec2 dzSample = permutation * imageLoad(dZ, index).xy / float(N);
    imageStore(dZ, index, vec4(dzSample.x, dzSample.y, 0.0, 0.0));
}