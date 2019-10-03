#version 430

layout(location=0) uniform vec4      info; // xy
vec2 uvOffset = info.xy;
vec2 invResolution = info.zw;

layout(binding=0)  uniform sampler2D texLinearDepth;

layout(location=0,index=0) out float out_Color[8];

//----------------------------------------------------------------------------------

#if 1
void main() {
  vec2 uv = floor(gl_FragCoord.xy) * 4.0 + uvOffset + 0.5;
  uv *= invResolution;  
  
  vec4 S0 = textureGather(texLinearDepth, uv, 0);
  vec4 S1 = textureGatherOffset(texLinearDepth, uv, ivec2(2,0), 0);
 
  out_Color[0] = S0.w;
  out_Color[1] = S0.z;
  out_Color[2] = S1.w;
  out_Color[3] = S1.z;
  out_Color[4] = S0.x;
  out_Color[5] = S0.y;
  out_Color[6] = S1.x;
  out_Color[7] = S1.y;
}
#else
void main() {
  vec2 uv = floor(gl_FragCoord.xy) * 4.0 + uvOffset;
  ivec2 tc = ivec2(uv);

  out_Color[0] = texelFetchOffset(texLinearDepth, tc, 0, ivec2(0,0)).x;
  out_Color[1] = texelFetchOffset(texLinearDepth, tc, 0, ivec2(1,0)).x;
  out_Color[2] = texelFetchOffset(texLinearDepth, tc, 0, ivec2(2,0)).x;
  out_Color[3] = texelFetchOffset(texLinearDepth, tc, 0, ivec2(3,0)).x;
  out_Color[4] = texelFetchOffset(texLinearDepth, tc, 0, ivec2(0,1)).x;
  out_Color[5] = texelFetchOffset(texLinearDepth, tc, 0, ivec2(1,1)).x;
  out_Color[6] = texelFetchOffset(texLinearDepth, tc, 0, ivec2(2,1)).x;
  out_Color[7] = texelFetchOffset(texLinearDepth, tc, 0, ivec2(3,1)).x;
}

#endif
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