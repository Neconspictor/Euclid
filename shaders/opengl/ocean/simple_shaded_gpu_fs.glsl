#version 450


in VS_OUT {
    vec3 normal;
    vec3 positionView;
    //vec2 texCoords;
} vs_out;

//in vec2 texCoord_tcs_in;

layout(location = 0)out vec4 fragColor;
layout(location = 1)out vec4 luminance;
layout(location = 2)out vec2 motion;
layout(location = 3)out float depth;

uniform vec3 lightDirViewSpace;
uniform mat3 normalMatrix;

void main() {

   vec3 normal = normalize(vs_out.normal);
   vec3 lightDir = normalize(-lightDirViewSpace);
   
   vec3 halfway = normalize(lightDir + normalize(-vs_out.positionView.xyz));

  float angle = max(dot(normal, lightDir), 0.0);
  
  
  
  
  
    vec4 c = vec4(1,1,1,1);//texture(water, tex_coord);
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

	fragColor.a = 1.0;
  
  
  
  
  
  
  
  
  
  //color =  angle * vec4(0.3, 1.0, 1.0, 1.0) + vec4(0.03, 0.1, 0.1, 0.0);
  luminance = vec4(0.0);
  motion = vec2(0.0);
  depth = gl_FragCoord.z;
}