#version 460 core

#include "util/depth_util.glsl"

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


void main()
{

    vec2 texCoord = gl_FragCoord.xy/viewport;
    float depthValue = texture(depth, texCoord).r;
    mat4 invView = inverse(viewMatrix);
    
    //float linearDepth = reconstructVSZ(depth);
    vec3 targetPositionVS = vec3(invView * vec4(reconstructPositionFromDepth(inverseProjMatrix, texCoord, depthValue), 1.0)); //linearDepth * normalize(positionVS);
    vec3 fragmentPositionVS = vec3(invView * vec4(reconstructPositionFromDepth(inverseProjMatrix, texCoord, gl_FragCoord.z), 1.0));
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