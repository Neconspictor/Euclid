#version 460 core

#define GROUP_NUM_X 1
#define GROUP_NUM_Y 1

layout (local_size_x = GROUP_NUM_X, local_size_y = GROUP_NUM_Y) in;

// readonly
layout (rg32f, binding = 0) uniform image2D source;

// writeonly
layout (rg32f, binding = 1) uniform image2D dest;

void main(void)
{
    const ivec2 index = ivec2(gl_GlobalInvocationID.xy);
    imageStore(dest, index, imageLoad(source, index));
}