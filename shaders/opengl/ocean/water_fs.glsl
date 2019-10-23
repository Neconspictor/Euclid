#version 450

#ifndef CSM_CASCADE_BUFFER_BINDING_POINT
#define CSM_CASCADE_BUFFER_BINDING_POINT 0
#endif

#ifndef CSM_CASCADE_DEPTH_MAP_BINDING_POINT
#define CSM_CASCADE_DEPTH_MAP_BINDING_POINT 8
#endif


in VS_OUT {
    vec3 normal;
    vec3 positionView;
    vec3 positionWorld;
    vec4 positionCS;
    vec2 texCoords;
} vs_out;

//in vec2 texCoord_tcs_in;

layout(location = 0)out vec4 fragColor;
layout(location = 1)out vec4 luminance;
//layout(location = 2)out vec2 motion;
//layout(location = 3)out float depth;

layout(binding = 5) uniform sampler2D colorMap;
layout(binding = 6) uniform sampler2D luminanceMap;
layout(binding = 7) uniform sampler2D depthMap;
//layout(binding = 7) uniform sampler2D reflectionMap;

uniform vec3 lightDirViewSpace;
uniform mat3 normalMatrix;
uniform mat4 inverseViewProjMatrix;

#include "shadow/cascaded_shadow.glsl"


float getLuma(in vec3 rgb) {
    return 1.0 * rgb.r + 1.0 * rgb.g + 1.0 * rgb.b; 
}


vec3 computeWorldPositionFromDepth(in vec2 texCoord, in float depth) {
  vec4 clipSpaceLocation;
  clipSpaceLocation.xy = texCoord * 2.0f - 1.0f;
  clipSpaceLocation.z = depth * 2.0f - 1.0f;
  clipSpaceLocation.w = 1.0f;
  vec4 homogenousLocation = inverseViewProjMatrix * clipSpaceLocation;
  return homogenousLocation.xyz / homogenousLocation.w;
};


vec4 calcAmbientColor(in vec3 normal, in vec3 halfway, in float angle) 
{
    vec4 c = vec4(1,1,1,1);
    float fog_factor = 0.0;

	vec4 emissive_color = vec4(1.0, 1.0, 1.0,  1.0);
	vec4 ambient_color  = vec4(0.0, 0.65, 0.75, 1.0);
	vec4 diffuse_color  = vec4(0.5, 0.65, 0.75, 1.0);
	vec4 specular_color = vec4(0.1, 0.5, 1.0,  1.0);

	float emissive_contribution = 0.00;
	float ambient_contribution  = 0.30;
	float diffuse_contribution  = 0.3;
	float specular_contribution = 1.80;

	bool facing = angle > 0.0;

	vec4 colorOut = emissive_color * emissive_contribution +
		    ambient_color  * ambient_contribution  * c +
		    diffuse_color  * diffuse_contribution  * c * angle +
                    (facing ?
			specular_color * specular_contribution * c * max(pow(dot(normal, halfway), 70.0), 0.0) :
			vec4(0.0, 0.0, 0.0, 0.0));

	return colorOut * (1.0-fog_factor) + vec4(0.25, 0.75, 0.65, 1.0) * (fog_factor);
}

float calcMurkiness(in float waterY, in float groundY, in float maxY) {
    float murk = waterY - groundY;
    
    murk = clamp(murk, 0.0, maxY);
    
    // normalize Note: if maxY is zero we have to clamp!    
    murk = clamp(murk / maxY, 0.0, 1.0);
    
    return murk;
}


void main() {

    vec3 normal = normalize(vs_out.normal);
    vec3 lightDir = normalize(-lightDirViewSpace);
    vec3 halfway = normalize(lightDir + normalize(-vs_out.positionView.xyz));
    float angle = max(dot(normal, lightDir), 0.0);
    float fragmentLitProportion = cascadedShadow(lightDir, normal, vs_out.positionView.z, vs_out.positionView);
    const float refractionRatio = 1.0 / 1.3;
    vec4 murkColor = calcAmbientColor(normal, halfway, angle);
  
    // uvs
    vec2 uv = vs_out.positionCS.xy / vs_out.positionCS.w;
    vec2 refractionUV = vs_out.positionCS.xy;
    refractionUV.x += normal.x * refractionRatio * 0.05;
    refractionUV.y += normal.z * refractionRatio * 0.05;
    //refractionUV.xy = clamp(refractionUV.xy, 0.0, 1.0);
    refractionUV= refractionUV.xy / vs_out.positionCS.w;
    refractionUV = clamp(refractionUV, 0.0, 1.0);
    
    
    // colors
    vec3 nonRefractedColor = texture(colorMap, uv).rgb;
    vec4 refractionColor = texture(colorMap, refractionUV);
    
    //water depths;
    float refractionDepth = texture(depthMap, refractionUV).r;
    float waterHeightDepth = gl_FragDepth;
    float refractionY = computeWorldPositionFromDepth(refractionUV, refractionDepth).y;
    float waterY = computeWorldPositionFromDepth(uv, waterHeightDepth).y;
    
    //calculate murkiness
    float murk = calcMurkiness(waterY, refractionY, 1.0);
    
  
  
    //illumination
    float litLuma = clamp(getLuma(refractionColor.rgb), 0.0, 1.0);
    
    // final color output
    fragColor = mix(fragColor, refractionColor, 1.0);
    //fragColor.rgb = mix(fragColor.rgb, murkColor.rgb, murk);
    
    
    fragColor = mix(fragColor, vec4(1.0, 1.0, 1.0, fragColor.a), 0.1);
    fragColor.rgb *= litLuma * vec3(1.0, 0.9, 0.7);
  
    luminance = texture(luminanceMap, refractionUV);
}