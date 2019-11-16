#version 330 core
out float FragColor;

in vec2 TexCoords;

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D texNoise;

uniform vec3 samples[64];

// parameters (you'd probably want to use them as uniforms to more easily tweak the effect)
int kernelSize = 64;
float radius = 0.05;
float bias = 0.025;


//const vec2 noiseScale = vec2(1920.0/4.0, 1080.0/4.0); 

uniform mat4 projection_GPass;


vec3 getPosition(vec2 uv) 
{
  return texture(gPosition, TexCoords).xyz;
}

vec3 getNormal(vec2 uv) 
{
  return normalize(texture(gNormal, TexCoords).rgb);
}

vec2 getRandom(vec2 uv) 
{
	// tile noise texture over screen based on screen dimensions divided by noise size
	vec2 noiseScale = textureSize(gPosition, 0) / 4.0;
	return normalize(texture(texNoise, TexCoords * noiseScale).xyz);
}

float doAmbientOcclusion(vec2 tcoord,vec2 uv,vec3 p,vec3 cnorm) 
{
  vec3 diff = getPosition(tcoord + uv) - p; 
  const vec3 v = normalize(diff); 
  const float d = length(diff) * 1; 
  return max(0.0,dot(cnorm,v) - bias)*(1.0/(1.0+d))* 2.2;
}

void main() 
{
  const vec2 vec[4] = {vec2(1,0),vec2(-1,0), vec2(0,1),vec2(0,-1)};
  vec3 position = getPosition(TexCoords); 
  vec3 normal = getNormal(TexCoords); 
  vec2 randomVec = getRandom(TexCoords); 
  float ao = 0.0f; 
  float specificRadius = radius/position.z; 
  
  //**SSAO Calculation**// 
  int iterations = 4; 
  for (int i = 0; i < iterations; ++i) 
  {
    vec2 coord1 = reflect(vec[i], randomVec) * specificRadius; 
    vec2 coord2 = vec2(coord1.x*0.707 - coord1.y*0.707, 
					   coord1.x*0.707 + coord1.y*0.707); 
    
    ao += doAmbientOcclusion(TexCoords,coord1*0.25, position, normal); 
    ao += doAmbientOcclusion(TexCoords,coord2*0.5, position, normal); 
    ao += doAmbientOcclusion(TexCoords,coord1*0.75, position, normal); 
    ao += doAmbientOcclusion(TexCoords,coord2, position, normal); 
  }
  
  ao/=(float)iterations*4.0; 
  
  FragColor = ao;
}