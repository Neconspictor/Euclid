#version 430 core

layout (local_size_x = 1, local_size_y = 1) in;

layout(r32ui, binding = 0) image1D waterMinDepths;

void main(void)
{   
    imageStore(waterMinDepths, gl_GlobalInvocationID.y, floatBitsToUint(1.0));
}