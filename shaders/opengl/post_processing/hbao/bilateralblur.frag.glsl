#version 460 core

const float KERNEL_RADIUS = 3;
  
uniform float g_Sharpness;
uniform vec2  g_InvResolutionDirection; // either set x to 1/width or y to 1/height

layout(binding=0) uniform sampler2D texSource;
layout(binding=1) uniform sampler2D texLinearDepth;

in vec2 texCoord;

layout(location=0,index=0) out vec4 out_Color;


//-------------------------------------------------------------------------

vec4 BlurFunction(vec2 uv, float r, vec4 center_c, float center_d, inout float w_total)
{
  vec4  c = texture2D( texSource, uv );
  float d = texture2D( texLinearDepth, uv).x;
  
  const float BlurSigma = float(KERNEL_RADIUS) * 0.5;
  const float BlurFalloff = 1.0 / (2.0*BlurSigma*BlurSigma);
  
  float ddiff = (d - center_d) * g_Sharpness;
  float w = exp2(-r*r*BlurFalloff - ddiff*ddiff);
  w_total += w;

  return c*w;
}

void main()
{
  vec4  center_c = texture2D( texSource, texCoord );
  float center_d = texture2D( texLinearDepth, texCoord).x;
  
  vec4  c_total = center_c;
  float w_total = 1.0;
  
  for (float r = 1; r <= KERNEL_RADIUS; ++r)
  {
    vec2 uv = texCoord + g_InvResolutionDirection * r;
    c_total += BlurFunction(uv, r, center_c, center_d, w_total);  
  }
  
  for (float r = 1; r <= KERNEL_RADIUS; ++r)
  {
    vec2 uv = texCoord - g_InvResolutionDirection * r;
    c_total += BlurFunction(uv, r, center_c, center_d, w_total);  
  }

  out_Color = c_total/w_total;
}