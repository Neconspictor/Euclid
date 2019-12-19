#version 460 core

in vec2 texCoord;

layout(location=0) uniform vec4 clipInfo; // z_n * z_f,  z_n - z_f,  z_f, perspective = 1 : 0

layout(location=1, binding=0)  uniform sampler2D inputTexture;

layout(location=0,index=0) out float out_Color;

#include "util/util.glsl"


/**
 * 
 */
float getDepth() {
 // Note: we render in halfth resolution, so we fetch 4 samples an average

 float samples = texelFetch(inputTexture, ivec2(2.0* gl_FragCoord.xy), 0).x;
 samples += texelFetch(inputTexture, ivec2(2.0* gl_FragCoord.xy) + ivec2(1,0), 0).x;
 samples += texelFetch(inputTexture, ivec2(2.0* gl_FragCoord.xy) + ivec2(0,1), 0).x;
 samples += texelFetch(inputTexture, ivec2(2.0* gl_FragCoord.xy) + ivec2(1,1), 0).x;
 
 return samples / 4.0;
}

float getDepth2() {
 // Note: we render in halfth resolution, so we fetch 4 samples an average

 float samples = texture(inputTexture, texCoord).r;
 
 return samples;
}

void main() {  
  float depth = getDepth2(); 

  out_Color = reconstructViewSpaceZ(depth, clipInfo);
}