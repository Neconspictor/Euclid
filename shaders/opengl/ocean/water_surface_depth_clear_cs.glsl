#version 430 core

layout (local_size_x = 1) in;

layout(r32ui, binding = 0) uniform uimage1D waterMinDepths;

void main(void)
{   
    imageStore(waterMinDepths, int(gl_GlobalInvocationID.x), uvec4(floatBitsToUint(1.0)));
}