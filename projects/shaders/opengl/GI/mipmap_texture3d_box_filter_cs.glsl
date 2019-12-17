#version 460 core

layout (local_size_x = 1, local_size_y = 1, local_size_z = 1) in;

layout (rgba32f, binding = 0) uniform image3D source;
layout (rgba32f, binding = 1) uniform image3D dest;


void main()
{  
   ivec3 baseID = ivec3(gl_GlobalInvocationID) * 2;
   
   vec4 sample0 = imageLoad(source, baseID);
   vec4 sample1 = imageLoad(source, baseID + ivec3(1,0,0));
   vec4 sample2 = imageLoad(source, baseID + ivec3(0,1,0));
   vec4 sample3 = imageLoad(source, baseID + ivec3(0,0,1));
   
   vec4 sample4 = imageLoad(source, baseID + ivec3(1,1,0));
   vec4 sample5 = imageLoad(source, baseID + ivec3(1,0,1));
   vec4 sample6 = imageLoad(source, baseID + ivec3(0,1,1));
   vec4 sample7 = imageLoad(source, baseID + ivec3(1,1,1));
   
   vec4 result = (sample0 + sample1 + sample2 + sample3 + sample4 + sample5 + sample6 + sample7) / 8.0;
    
    imageStore(dest, ivec3(gl_GlobalInvocationID) , result);
}