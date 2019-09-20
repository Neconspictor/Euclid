#ifndef VOXEL_UTIL_GLSL
#define VOXEL_UTIL_GLSL

struct VoxelType
{
	uint colorMask;
	uint normalMask;
};

const float HDR_RANGE = 10.0f;

bool is_saturated(float a) { return a == clamp(a, 0.0, 1.0); }
bool is_saturated(vec2 a) { return is_saturated(a.x) && is_saturated(a.y); }
bool is_saturated(vec3 a) { return is_saturated(a.x) && is_saturated(a.y) && is_saturated(a.z); }
bool is_saturated(vec4 a) { return is_saturated(a.x) && is_saturated(a.y) && is_saturated(a.z) && is_saturated(a.w); }

// 3D array index to flattened 1D array index
uint flatten3D(uvec3 coord, uvec3 dim)
{
	return (coord.z * dim.x * dim.y) + (coord.y * dim.x) + coord.x;
}
// flattened array index to 3D array index
uvec3 unflatten3D(uint idx, uvec3 dim)
{
	const uint z = idx / (dim.x * dim.y);
	idx -= (z * dim.x * dim.y);
	const uint y = idx / dim.x;
	const uint x = idx % dim.x;
	return  uvec3(x, y, z);
}


// Encode HDR color to a 32 bit uint
// Alpha is 1 bit + 7 bit HDR remapping
uint EncodeColor(in vec4 color)
{
	// normalize color to LDR
	float hdr = length(color.rgb);
	color.rgb /= hdr;

	// encode LDR color and HDR range
	uvec3 iColor = uvec3(color.rgb * 255.0f);
	uint iHDR = uint(clamp(hdr / HDR_RANGE, 0.0, 1.0) * 127);
	uint colorMask = (iHDR << 24u) | (iColor.r << 16u) | (iColor.g << 8u) | iColor.b;

	// encode alpha into highest bit
	uint iAlpha = (color.a > 0 ? 1u : 0u);
	colorMask |= iAlpha << 31u;

	return colorMask;
}

// Decode 32 bit uint into HDR color with 1 bit alpha
vec4 DecodeColor(in uint colorMask)
{
	float hdr;
	vec4 color;

	hdr = (colorMask >> 24u) & 0x0000007f;
	color.r = (colorMask >> 16u) & 0x000000ff;
	color.g = (colorMask >> 8u) & 0x000000ff;
	color.b = colorMask & 0x000000ff;

	hdr /= 127.0f;
	color.rgb /= 255.0f;

	color.rgb *= hdr * HDR_RANGE;

	color.a = (colorMask >> 31u) & 0x00000001;

	return color;
}

// Encode specified normal (normalized) into an unsigned integer. Each axis of
// the normal is encoded into 9 bits (1 for the sign/ 8 for the value).
uint EncodeNormal(in vec3 normal)
{
	ivec3 iNormal = ivec3(normal*255.0f);
	uvec3 iNormalSigns;
	iNormalSigns.x = (iNormal.x >> 5) & 0x04000000;
	iNormalSigns.y = (iNormal.y >> 14) & 0x00020000;
	iNormalSigns.z = (iNormal.z >> 23) & 0x00000100;
	iNormal = abs(iNormal);
	uint normalMask = iNormalSigns.x | (iNormal.x << 18) | iNormalSigns.y | (iNormal.y << 9) | iNormalSigns.z | iNormal.z;
	return normalMask;
}

// Decode specified mask into a float3 normal (normalized).
vec3 DecodeNormal(in uint normalMask)
{
	ivec3 iNormal;
	iNormal.x = int((normalMask >> 18) & 0x000000ff);
	iNormal.y = int((normalMask >> 9) & 0x000000ff);
	iNormal.z = int(normalMask & 0x000000ff);
	ivec3 iNormalSigns;
	iNormalSigns.x = int((normalMask >> 25) & 0x00000002);
	iNormalSigns.y = int((normalMask >> 16) & 0x00000002);
	iNormalSigns.z = int((normalMask >> 7) & 0x00000002);
	iNormalSigns = 1 - iNormalSigns;
	vec3 normal = vec3(iNormal) / 255.0f;
	normal *= iNormalSigns;
	return normal;
}

#endif