#version 460 core

#ifndef KERNEL_RADIUS
#define KERNEL_RADIUS 3
#endif
  
uniform float g_Sharpness;
uniform vec2  g_InvResolutionDirection; // either set x to 1/width or y to 1/height

layout(binding=0) uniform sampler2D texSource;
layout(binding=1) uniform sampler2D texViewSpaceZ;

in VS_OUT {
    vec2 texCoord;
} fs_in;

layout(location=0,index=0) out vec4 out_Color;


//-------------------------------------------------------------------------

vec4 BlurFunction(vec2 uv, float r, vec4 center_c, float center_d, inout float w_total)
{
  vec4  c = texture2D( texSource, uv );
  float d = abs(texture2D( texViewSpaceZ, uv).x);
  
  const float BlurSigma = float(KERNEL_RADIUS) * 0.5;
  const float BlurFalloff = 1.0 / (2.0*BlurSigma*BlurSigma);
  
  float ddiff = (d - center_d) * g_Sharpness;
  float w = exp2(-r*r*BlurFalloff - ddiff*ddiff);
  w_total += w;

  return c*w;
}

void main()
{
  vec4  center_c = texture2D( texSource, fs_in.texCoord );
  float center_d = abs(texture2D( texViewSpaceZ, fs_in.texCoord).x);
  
  vec4  c_total = center_c;
  float w_total = 1.0;
  
  for (float r = 1; r <= KERNEL_RADIUS; ++r)
  {
    vec2 uv = fs_in.texCoord + g_InvResolutionDirection * r;
    c_total += BlurFunction(uv, r, center_c, center_d, w_total);  
  }
  
  for (float r = 1; r <= KERNEL_RADIUS; ++r)
  {
    vec2 uv = fs_in.texCoord - g_InvResolutionDirection * r;
    c_total += BlurFunction(uv, r, center_c, center_d, w_total);  
  }

  out_Color = c_total/w_total;
}