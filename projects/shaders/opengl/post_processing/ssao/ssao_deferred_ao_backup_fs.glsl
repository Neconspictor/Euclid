#version 460 core
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

void main()
{

	// tile noise texture over screen based on screen dimensions divided by noise size
	vec2 noiseScale = textureSize(gPosition, 0) / 4.0;

    // get input for SSAO algorithm
    vec3 fragPos = texture(gPosition, TexCoords).xyz;
    vec3 normal = normalize(texture(gNormal, TexCoords).rgb);
	
	/*float nLength = length(normal);
	if (nLength < 0.9) {
		discard;
	}*/
	
    vec3 randomVec = normalize(texture(texNoise, TexCoords * noiseScale).xyz);
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
    for(int i = 0; i < kernelSize; ++i)
    {
        // get sample position
        vec3 sample = TBN * samples[i]; // from tangent to view-space
        sample = fragPos + sample * radius; 
        
        // project sample position (to sample texture) (to get position on screen/texture)
        vec4 offset = vec4(sample, 1.0);
        offset = projection_GPass * offset; // from view to clip-space
        offset.xyz /= offset.w; // perspective divide
        offset.xyz = offset.xyz * 0.5 + 0.5; // transform to range 0.0 - 1.0
        
        // get sample depth
        float sampleDepth = texture(gPosition, offset.xy).z; // get depth value of kernel sample
        
        // range check & accumulate
        float rangeCheck = smoothstep(0.0, 1.0, radius / abs(fragPos.z - sampleDepth));
		//float rangeCheck = 1.0;
	   occlusion += (sampleDepth >= sample.z + bias ? 1.0 : 0.0) * rangeCheck;           
    }
    occlusion = 1.0 - (occlusion / kernelSize);
	
	if (occlusion < 0.9) {
		occlusion = 0.0;
	}
	//occlusion = clamp(pow(occlusion, 2.2 * 2), 0, 1);
	
    FragColor = occlusion;
}