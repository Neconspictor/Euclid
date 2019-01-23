#version 430

#define SSAO_KERNEL_SIZE 32

struct SSAOData {
  float   bias;
  float   intensity;
  float   radius;
  float   _pad0; 
  
  mat4 projection_GPass;
  
  vec4 samples[SSAO_KERNEL_SIZE]; // the w component is not used (just for padding)!
};


layout(location=0,index=0) out float FragColor;

in vec2 texCoord;

layout(std140,binding=0) uniform controlBuffer {
  SSAOData   control;
};

layout(binding=0) uniform sampler2D gNormal;
layout(binding=1) uniform sampler2D gDepth;
layout(binding=2) uniform sampler2D texNoise;

// parameters (you'd probably want to use them as uniforms to more easily tweak the effect)
//float radius = 0.25;
//float bias = 0.025;


//const vec2 noiseScale = vec2(1920.0/4.0, 1080.0/4.0); 

vec3 computeViewPositionFromDepth(in vec2 texCoord, in float depth, in mat4 inverseMatrix) {
  vec4 clipSpaceLocation;
  clipSpaceLocation.xy = texCoord * 2.0f - 1.0f;
  clipSpaceLocation.z = depth * 2.0f - 1.0f;
  clipSpaceLocation.w = 1.0f;
  vec4 homogenousLocation = inverseMatrix * clipSpaceLocation;
  return homogenousLocation.xyz / homogenousLocation.w;
};

void main()
{

	// tile noise texture over screen based on screen dimensions divided by noise size
	vec2 noiseScale = textureSize(gDepth, 0) / 4.0;

    // get input for SSAO algorithm
    mat4 inverseProj = inverse(control.projection_GPass);
    vec3 fragPos = computeViewPositionFromDepth(texCoord, texture(gDepth, texCoord).r, inverseProj);
    vec3 normal = normalize(texture(gNormal, texCoord).rgb);
	
	//float nLength = length(normal);
	//if (nLength < 0.9) {
	//	discard;
	//}
	
    vec3 randomVec = normalize(texture(texNoise, texCoord * noiseScale).xyz);
    // create TBN change-of-basis matrix: from tangent-space to view-space
    vec3 tangent = normalize(randomVec - normal * dot(randomVec, normal));
	//vec3 tangent = normalize(randomVec);
	//vec3 tangent = normalize(vec3(0,1,0) + randomVec);
	//vec3 tangent = normalize(vec3(0, 0, 1));
	//tangent = normal;
    vec3 bitangent = cross(normal, tangent);
    mat3 TBN = mat3(tangent, bitangent, normal);
    // iterate over the sample kernel and calculate occlusion factor
    float occlusion = 0.0;
    for(int i = 0; i < SSAO_KERNEL_SIZE; ++i)
    {
        // get sample position
		
		
        vec3 sampleTest = TBN * control.samples[i].xyz; // from tangent to view-space
        sampleTest = fragPos + sampleTest * control.radius; 
        
        // project sample position (to sample texture) (to get position on screen/texture)
        vec4 offset = vec4(sampleTest, 1.0);
        offset = control.projection_GPass * offset; // from view to clip-space
        offset.xyz /= offset.w; // perspective divide
        offset.xyz = offset.xyz * 0.5 + 0.5; // transform to range 0.0 - 1.0
        
        // get sample depth
        //float sampleDepth = texture(gDepth, offset.xy).r; // get depth value of kernel sample
        float sampleDepth = computeViewPositionFromDepth(offset.xy, texture(gDepth, offset.xy).r, inverseProj).z;
        
        // range check & accumulate
        //float rangeCheck = smoothstep(0.0, 1.0, radius / abs(fragPos.z - sampleDepth));
		//float rangeCheck = 1.0;
		//occlusion += (sampleDepth >= sample.z + bias ? 1.0 : 0.0) * 1;  

		
		float delta =  sampleDepth - (sampleTest.z + control.bias);
		float rangeCheck = smoothstep(0.0, 1.0, control.radius / abs(delta));
		
		// If scene fragment is before (smaller in z) sample point, increase occlusion.
		if(delta > 0.0001) {
			occlusion += 1.0 * rangeCheck;
		};
	  
    }
	
	occlusion /= SSAO_KERNEL_SIZE;
	
	
	
	occlusion = clamp(pow(occlusion, 1/control.intensity), 0, 1);
	//occlusion = clamp(2.0 * occlusion, 0, 1);
    occlusion = 1.0 - (occlusion);
	//occlusion = clamp(pow(occlusion, 2.2), 0, 1);
	//if (occlusion < 0.99) {
	//	occlusion = clamp(pow(occlusion, 2.2), 0, 1);
	//}
	//occlusion = clamp(2.0 * occlusion, 0, 1);
	//occlusion = clamp(pow(occlusion, 2.2 * 2), 0, 1);
	
    FragColor = occlusion;
}