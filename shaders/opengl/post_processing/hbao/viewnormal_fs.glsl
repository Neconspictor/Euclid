#version 430

in vec2 texCoord;

layout(location=0) uniform vec4 projInfo; 
layout(location=1) uniform int  projOrtho;
layout(location=2) uniform vec2 InvFullResolution;

layout(binding=0)  uniform sampler2D texLinearDepth;

layout(location=0,index=0) out vec4 out_Color;

//----------------------------------------------------------------------------------

vec3 UVToView(vec2 uv, float eye_z)
{
  return vec3((uv * projInfo.xy + projInfo.zw) * (projOrtho != 0 ? 1. : (-eye_z)), eye_z);
}

vec3 FetchViewPos(vec2 UV)
{
  float ViewDepth = textureLod(texLinearDepth,UV,0).x;
  return UVToView(UV, ViewDepth);
}

vec3 MinDiff(vec3 P, vec3 Pr, vec3 Pl)
{
  vec3 V1 = Pr - P;
  vec3 V2 = P - Pl;
  return (dot(V1,V1) < dot(V2,V2)) ? V1 : V2;
}

vec3 ReconstructNormal(vec2 UV, vec3 P)
{
  vec3 Pr = FetchViewPos(UV + vec2(InvFullResolution.x, 0));
  vec3 Pl = FetchViewPos(UV + vec2(-InvFullResolution.x, 0));
  vec3 Pt = FetchViewPos(UV + vec2(0, InvFullResolution.y));
  vec3 Pb = FetchViewPos(UV + vec2(0, -InvFullResolution.y));
  
  return normalize(cross(MinDiff(P, Pr, Pl), MinDiff(P, Pt, Pb)));
}



vec3 FetchViewPosFromDepth(vec2 UV)
{
  float depth = textureLod(texLinearDepth,UV,0).x;
  
  float z_ndc = 2.0 * depth - 1.0;
  float z_eye = projInfo.w / (projInfo.z + z_ndc);
  
  float ndc_x = UV.x * 2.0 - 1.0;
  float ndc_y = UV.y * 2.0 - 1.0;
  
 return vec3(z_eye * ndc_x / projInfo.x, 
                      z_eye * ndc_y / projInfo.y, 
                      -z_eye);
}

vec3 ReconstructNormalFromDepth(vec2 UV, vec3 P)
{
  vec3 Pr = FetchViewPosFromDepth(UV + vec2(InvFullResolution.x, 0));
  vec3 Pl = FetchViewPosFromDepth(UV + vec2(-InvFullResolution.x, 0));
  vec3 Pt = FetchViewPosFromDepth(UV + vec2(0, InvFullResolution.y));
  vec3 Pb = FetchViewPosFromDepth(UV + vec2(0, -InvFullResolution.y));
  
  return normalize(cross(Pr - P, Pt - P));
}

//----------------------------------------------------------------------------------

void main() {
  //vec3 P  = FetchViewPosFromDepth(texCoord);
  //vec3 N  = ReconstructNormalFromDepth(texCoord, P);
  
  vec3 P  = FetchViewPos(texCoord);
  vec3 N  = ReconstructNormal(texCoord, P);
  
  out_Color = vec4(N*0.5 + 0.5,0);
}