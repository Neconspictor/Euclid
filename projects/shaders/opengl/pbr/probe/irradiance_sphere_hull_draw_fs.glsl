#version 460 core

#include "util/depth_util.glsl"

layout(location = 0) out vec4 color;
layout(location = 1) out vec4 luminance;
  
uniform vec4 objectColor;
uniform vec3 probePositionWS;
uniform float probeRadius;
uniform vec2 viewport;
uniform vec3 clipInfo;
uniform mat4 inverseViewProjMatrix;

layout(binding = 0) uniform sampler2D depth;

in vec3 positionWS;


void main()
{

    vec2 texCoord = gl_FragCoord.xy/viewport;
    float depthValue = texture(depth, texCoord).r;
    
    vec3 targetPositionWS = vec3(vec4(reconstructPositionFromDepth(inverseViewProjMatrix, texCoord, depthValue), 1.0));
    vec3 fragmentPositionWS = vec3(vec4(reconstructPositionFromDepth(inverseViewProjMatrix, texCoord, gl_FragCoord.z), 1.0));
    
    float maxDist = distance(probePositionWS, fragmentPositionWS);
    maxDist = probeRadius;
    float dist = distance(probePositionWS, targetPositionWS);
    
    if (dist > maxDist) 
    {
        discard;
    }

    color = objectColor;
	luminance = vec4(0.0);
}