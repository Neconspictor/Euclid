#version 460 core

#include "interface/buffers.h"

in VS_OUT {
    vec3 fragment_position_eye;
	vec3 normalEye;
    vec2 texCoords;
} fs_in;


layout(location = 0) out vec4 FragColor;
layout(location = 1) out vec4 LuminanceColor;

layout(binding = 0) uniform samplerCubeArray probes;
uniform float arrayIndex;

void main()
{    		
    // view direction
    vec3 positionEye = fs_in.fragment_position_eye.rgb;
	vec3 viewEye = normalize(-positionEye);
    
	// reflection direction
    vec3 viewWorld = vec3(constants.invViewGPass * vec4(viewEye, 0.0f));
	vec3 normalWorld = vec3(constants.invViewGPass * vec4(fs_in.normalEye, 0.0f));
    vec3 reflectionDirWorld = normalize(reflect(-viewWorld, normalWorld));    
    vec3 color = textureLod(probes, vec4(reflectionDirWorld, arrayIndex), 0.0).rgb;
        
    FragColor = vec4(color, 1.0);
    LuminanceColor = vec4(0,0,0,1.0);
}