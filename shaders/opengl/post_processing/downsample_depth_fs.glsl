#version 420

in vec2 texCoord;

out vec4 fragColor;

layout(binding = 0) uniform sampler2D sourceTexture;

void main() {

	float d1 = textureOffset(sourceTexture, texCoord, ivec2(0, 0)).x;
	float d2 = textureOffset(sourceTexture, texCoord, ivec2(0, 1)).x;
	float d3 = textureOffset(sourceTexture, texCoord, ivec2(1, 1)).x;
	float d4 = textureOffset(sourceTexture, texCoord, ivec2(1, 0)).x;
 
	fragColor = vec4(max(max(d1, d2), max(d3, d4)));
}