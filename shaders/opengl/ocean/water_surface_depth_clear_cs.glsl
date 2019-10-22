#version 430 core

layout (local_size_x = 1) in;

layout(r32i, binding = 0) uniform iimage1D waterMinDepths;
layout(r32i, binding = 1) uniform iimage1D waterMaxDepths;

void main(void)
{   
    imageStore(waterMinDepths, int(gl_GlobalInvocationID.x), ivec4(floatBitsToInt(1000000000.0)));
    imageStore(waterMaxDepths, int(gl_GlobalInvocationID.x), ivec4(floatBitsToInt(-1000000000.0)));
}