#version 430

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

/*-----------------------------------------------------------------------
  Copyright (c) 2014, NVIDIA. All rights reserved.

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions
  are met:
   * Redistributions of source code must retain the above copyright
     notice, this list of conditions and the following disclaimer.
   * Neither the name of its contributors may be used to endorse 
     or promote products derived from this software without specific
     prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS ``AS IS'' AND ANY
  EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
  PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR
  CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
  EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
  PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
  PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
  OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
-----------------------------------------------------------------------*/