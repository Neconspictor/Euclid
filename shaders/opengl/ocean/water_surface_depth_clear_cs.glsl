#version 430 core

layout (local_size_x = 1) in;

layout(r32i, binding = 0) uniform iimage1D waterMinDepths;

void main(void)
{   
    imageStore(waterMinDepths, int(gl_GlobalInvocationID.x), ivec4(floatBitsToInt(1000000000.0)));
}