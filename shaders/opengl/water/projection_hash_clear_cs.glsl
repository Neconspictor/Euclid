#version 450 core

layout (local_size_x = 1, local_size_y = 1) in;

// writeonly
layout (r32ui, binding = 0) uniform uimage2D projHashMap;

void main(void)
{
    const ivec2 pixelLoc = ivec2(gl_GlobalInvocationID.xy);    
    imageStore(projHashMap, pixelLoc, uvec4(0));
}