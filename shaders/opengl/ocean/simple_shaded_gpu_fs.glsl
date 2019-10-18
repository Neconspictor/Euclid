#version 450


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

uniform vec3 lightDirViewSpace;
uniform mat3 normalMatrix;

layout(binding = 5) uniform sampler2D colorMap;
layout(binding = 6) uniform sampler2D luminanceMap;
layout(binding = 7) uniform sampler2D depthMap;
//layout(binding = 7) uniform sampler2D reflectionMap;

void main() {

   vec3 normal = normalize(vs_out.normal);
   vec3 lightDir = normalize(-lightDirViewSpace);
   
   vec3 halfway = normalize(lightDir + normalize(-vs_out.positionView.xyz));

  float angle = max(dot(normal, lightDir), 0.0);
  
  
  
  
  
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

	fragColor = emissive_color * emissive_contribution +
		    ambient_color  * ambient_contribution  * c +
		    diffuse_color  * diffuse_contribution  * c * angle +
                    (facing ?
			specular_color * specular_contribution * c * max(pow(dot(normal, halfway), 70.0), 0.0) :
			vec4(0.0, 0.0, 0.0, 0.0));

	fragColor = fragColor * (1.0-fog_factor) + vec4(0.25, 0.75, 0.65, 1.0) * (fog_factor);
   
 
  
    vec2 ndcPos = vec2(vs_out.positionCS.xy / vs_out.positionCS.w);
    
    const float refractionRatio = 1.0 / 1.3;
    
    vec2 refractionNDC = ndcPos ;//+ normal.xy * refractionRatio;
    vec4 refractionUV = vs_out.positionCS;
    refractionUV.x += normal.x * refractionRatio * 0.1;
    refractionUV.y += normal.z * refractionRatio * 0.1;
    //refractionUV.xy = clamp(refractionUV.xy, 0.0, 1.0);
    vec2 uv = refractionUV.xy / refractionUV.w;
    uv = clamp(uv, 0.001, 0.999);
    vec4 refractionColor = texture(colorMap, uv);
    
    fragColor = mix(fragColor, refractionColor, 0.5);
    luminance = texture(luminanceMap, uv);
    
    
    //fragColor.a = 1.0;
  
    //float depth = texture2DProj(depthMap, vs_out.positionCS).r;
  
  
  
  
  //color =  angle * vec4(0.3, 1.0, 1.0, 1.0) + vec4(0.03, 0.1, 0.1, 0.0);
  //luminance = vec4(0.0);
  //motion = vec2(0.0);
  //depth = gl_FragCoord.z;
}