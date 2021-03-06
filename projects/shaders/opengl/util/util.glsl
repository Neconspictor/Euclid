#ifndef UTIL_HEADER
#define UTIL_HEADER


float rgb2luma(in vec3 color)
{
  const vec3 LUMA_CONVERSION = vec3(0.299, 0.587, 0.114);

  #ifdef SOURCE_GAMMA_SPACE
    return sqrt(dot(color, LUMA_CONVERSION));
  #else
    return dot(color, LUMA_CONVERSION);
  #endif
}

float rgb2luma(in vec4 color)
{
  return rgb2luma(color.rgb);
}

#endif
