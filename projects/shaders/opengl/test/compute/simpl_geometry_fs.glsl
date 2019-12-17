#version 460 core

layout(location = 0) out float depth;


void main()
{  
    depth = gl_FragCoord.z; 
}