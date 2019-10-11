#ifndef UTIL_HEADER
#define UTIL_HEADER

float rgb2luma(in vec4 color)
{
  const vec3 LUMA_CONVERSION = vec3(0.299, 0.587, 0.114);

  #ifdef SOURCE_GAMMA_SPACE
    return sqrt(dot(color.rgb, LUMA_CONVERSION));
  #else
    return dot(color.rgb, LUMA_CONVERSION);
  #endif
}


float reconstructViewSpaceZ(float d, vec4 clipInfo) {
  if (clipInfo[3] != 0) {
    return (clipInfo[0] / (clipInfo[1] * d + clipInfo[2]));
  }
  else {
    return (clipInfo[1]+clipInfo[2] - d * clipInfo[1]);
  }
}

#endif
