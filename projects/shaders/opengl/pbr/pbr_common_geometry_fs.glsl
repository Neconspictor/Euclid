#include "pbr/viewspaceNormalization.glsl"

struct Material {
    layout(binding = 0) sampler2D albedoMap;
	layout(binding = 1) sampler2D aoMap;
	layout(binding = 2) sampler2D metallicMap;
	layout(binding = 3) sampler2D normalMap;
	layout(binding = 4) sampler2D roughnessMap;
};


in VS_OUT {	
	vec4 fragment_position_eye;
	vec4 fragment_position_world;
	vec4 camera_position_world;
    //float viewSpaceZ;
    vec4 position_ndc;
    vec4 position_ndc_previous;
	vec2 tex_coords;
	mat3 TBN_eye_directions; // used to transform the normal vector from tangent to eye space.
						  //  This matrix mustn't be used with positions!!!
} fs_in;


uniform Material material;

// Viewspace z values of the near and far plane
// Note: near and far mustn't be equal (Divide by zero!)
uniform vec2 nearFarPlane;

vec3 getNormalEye() {
float factor = 255/128.0f;	// is better than 2.0f for precision reasons!
 vec3 normalTangent = (texture(material.normalMap, fs_in.tex_coords).xyz * factor) - 1.0;
	return normalize(fs_in.TBN_eye_directions * normalTangent);
}

vec3 getEncodedNormalEye() {
	return 0.5 * getNormalEye() + 0.5;
}