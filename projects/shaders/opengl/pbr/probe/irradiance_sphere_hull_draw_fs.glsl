#version 420 core

layout(location = 0) out vec4 color;
layout(location = 1) out vec4 luminance;
  
uniform vec4 objectColor;
uniform vec3 probePositionWS;
uniform float probeRadius;
uniform vec2 viewport;
uniform vec3 clipInfo;
uniform mat4 inverseProjMatrix;

layout(binding = 0) uniform sampler2D depth;

in vec3 positionWS;
in vec3 positionVS;
in mat4 viewMatrix;
in vec3 positionLS;

/** Given an OpenGL depth buffer value on [0, 1] and description of the projection
    matrix's clipping planes, computes the view-space (negative) z value.

    See also computeClipInfo in the .cpp file */ 
float reconstructVSZ(float depthBufferValue) {
      return clipInfo[0] / (depthBufferValue * clipInfo[1] + clipInfo[2]);
}

vec3 computeViewPositionFromDepth(in vec2 texCoord, in float depthBufferValue) {
  vec4 clipSpaceLocation;
  clipSpaceLocation.xy = texCoord * 2.0f - 1.0f;
  clipSpaceLocation.z = depthBufferValue * 2.0f - 1.0f;
  clipSpaceLocation.w = 1.0f;
  vec4 homogenousLocation = inverseProjMatrix * clipSpaceLocation;
  return homogenousLocation.xyz / homogenousLocation.w;
};


void main()
{

    vec2 texCoord = gl_FragCoord.xy/viewport;
    float depthValue = texture(depth, texCoord).r;
    mat4 invView = inverse(viewMatrix);
    
    //float linearDepth = reconstructVSZ(depth);
    vec3 targetPositionVS = vec3(invView * vec4(computeViewPositionFromDepth(texCoord, depthValue), 1.0)); //linearDepth * normalize(positionVS);
    vec3 fragmentPositionVS = vec3(invView * vec4(computeViewPositionFromDepth(texCoord, gl_FragCoord.z), 1.0));
    vec3 probePositionVS = probePositionWS;//(viewMatrix * vec4(probePositionWS, 1.0)).xyz;
    
    float maxDist = distance(probePositionVS, fragmentPositionVS);
    maxDist = probeRadius;
    float dist = distance(probePositionVS, targetPositionVS);
    
    if (dist > maxDist) 
    {
        discard;
    }
    
    /*if (targetPositionVS.z > fragmentPositionVS.z) {
        //gl_FragDepth = depthValue;
    }
    
    if (length(targetPositionVS) > length(probePositionVS)) {
    
    } else {
        discard;
    }
    
    if (depthValue == 1.0)  {
        discard;
    }*/

    color = objectColor;
	luminance = vec4(0.0);
}