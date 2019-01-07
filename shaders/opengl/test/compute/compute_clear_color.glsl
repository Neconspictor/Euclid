#version 430 core
layout (local_size_x = 1, local_size_y = 1) in;
// An image to store data into.
layout (rg32f, binding = 0) uniform image2D data;


void main(void)
{
    vec4 color = vec4(0.0,0,0,1);
    imageStore(data, ivec2(gl_GlobalInvocationID.xy), color);
}