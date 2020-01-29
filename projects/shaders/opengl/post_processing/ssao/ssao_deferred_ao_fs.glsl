#version 460 core

#define SSAO_KERNEL_SIZE 32

struct SSAOData {
  float   bias;
  float   intensity;
  float   radius;
  float   _pad0; 
  
  vec4 invFullResolution; //only x,y component used
  
  mat4 projection_GPass;
  mat4 inv_projection_GPass;
  
  vec4 samples[SSAO_KERNEL_SIZE]; // the w component is not used (just for padding)!
};


layout(location=0,index=0) out float FragColor;

in vec2 texCoord;

layout(std140,binding=0) uniform controlBuffer {
  SSAOData   control;
};

layout(binding=0) uniform sampler2D gDepth;
layout(binding=1) uniform sampler2D texNoise;
//layout(binding=2) uniform sampler2D gNormal;

// parameters (you'd probably want to use them as uniforms to more easily tweak the effect)
//float radius = 0.25;
//float bias = 0.025;


//const vec2 noiseScale = vec2(1920.0/4.0, 1080.0/4.0); 

/*vec3 computeViewPositionFromDepth(in vec2 texCoord, float depth) {
    
    vec2 positionNDC = texCoord * 2.0 - 1.0;
    
    // Unproject depth z value into view space
    float z_ndc = 2.0 * depth - 1.0;
    float viewSpaceZ =  control.projection_GPass[3][2] / (z_ndc + control.projection_GPass[2][2]); //TODO  
    
    // For more information: https://stackoverflow.com/questions/11277501/how-to-recover-view-space-position-given-view-space-depth-value-and-ndc-xy/46118945#46118945
    
    vec2 screenSpaceRay = vec2(
        viewSpaceZ * positionNDC.x / control.projection_GPass[0][0],
        viewSpaceZ * positionNDC.y / control.projection_GPass[1][1]);
                               
    vec3 positionView;
    positionView.z = -viewSpaceZ;
    positionView.xy = screenSpaceRay.xy;

    return positionView;
}*/


/*
vec3 UVToView(vec2 uv, float eye_z)
{
  return vec3((uv * control.projInfo.xy + control.projInfo.zw) * (control.projOrtho != 0 ? 1. : eye_z), eye_z);
}*/

vec3 fetchViewPos(vec2 uv)
{  
  float depth = texture(gDepth,uv).r;
  return computeViewPositionFromDepth(uv, depth);
}

vec3 minDiff(vec3 P, vec3 Pr, vec3 Pl)
{
  vec3 V1 = Pr - P;
  vec3 V2 = P - Pl;
  return (dot(V1,V1) < dot(V2,V2)) ? V1 : V2;
}

vec3 reconstructNormal(vec2 uv, vec3 position)
{
  vec2 invTexSize = control.invFullResolution.xy; //textureSize(gDepth, 0);

  vec3 right = fetchViewPos(uv + vec2(invTexSize.x, 0));
  vec3 left = fetchViewPos(uv + vec2(-invTexSize.x, 0));
  vec3 top = fetchViewPos(uv + vec2(0, invTexSize.y));
  vec3 bottom = fetchViewPos(uv + vec2(0, -invTexSize.y));
  
  return normalize(cross(minDiff(position, right, left), minDiff(position, top, bottom)));
}

void main()
{



    vec2 uv = texCoord;
    
    float depth = texture(gDepth, uv).r;
    
    //if (depth == 0.0) {
    //    discard;
    //};

	// tile noise texture over screen based on screen dimensions divided by noise size
	vec2 noiseScale = textureSize(gDepth, 0) / 4.0;

    // get input for SSAO algorithm
    vec3 fragPos = computeViewPositionFromDepth(uv, depth);
    vec3 normal = reconstructNormal(uv, fragPos);
    
	//float nLength = length(normal);
	//if (nLength < 0.9) {
	//	discard;
	//}
	
    vec3 randomVec = normalize(texture(texNoise, uv * noiseScale).xyz);
    // create TBN change-of-basis matrix: from tangent-space to view-space
    vec3 tangent = normalize(randomVec - normal * dot(randomVec, normal));
	//vec3 tangent = normalize(randomVec);
	//vec3 tangent = normalize(vec3(0,1,0) + randomVec);
	//vec3 tangent = normalize(vec3(0, 0, 1));
	//tangent = normal;
    vec3 bitangent = normalize(cross(normal, tangent));
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
        float sampleDepth = reconstructPositionFromDepth(control.inv_projection_GPass, offset.xy, texture(gDepth, offset.xy).r).z;
        
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
	
	
	
	occlusion = clamp(pow(occlusion, 1.0 / control.intensity), 0, 1);
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