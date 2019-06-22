#version 430

in VS_OUT {
    vec3 fragment_position_eye;
	vec3 normalEye;
    vec2 texCoords;
    mat4 inverseView;
} fs_in;


layout(location = 0) out vec4 FragColor;
layout(location = 1) out vec4 LuminanceColor;

layout(binding = 0) uniform samplerCube irradianceMap;
layout(binding = 1) uniform samplerCube prefilterMap;
// The inverse view matrix. Note, for deferred renderings the inverse view of the geometry pass is meant!
//uniform mat4 inverseViewMatrix;

void main()
{    		
    vec3 normalWorld = vec3(fs_in.inverseView * vec4(fs_in.normalEye, 0.0f));
    
    vec3 irradiance = texture(irradianceMap, normalWorld).rgb;
   // irradiance = vec3(1,0,0);
   
   
    // view direction
    vec3 positionEye = fs_in.fragment_position_eye.rgb;
	vec3 viewEye = normalize(-positionEye);
    
	// reflection direction
    vec3 viewWorld = vec3(fs_in.inverseView * vec4(viewEye, 0.0f));
    vec3 reflectionDirWorld = normalize(reflect(-viewWorld, normalWorld));
    
    vec3 prefiltered = textureLod(prefilterMap, reflectionDirWorld, 1.0).rgb;
        
    FragColor = vec4(prefiltered, 1.0);
    LuminanceColor = vec4(0,0,0,1.0);
    
}