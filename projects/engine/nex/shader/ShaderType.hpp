#pragma once

namespace nex
{
	enum class UniformType
	{
		CUBE_MAP,
		CUBE_MAP_ARRAY,
		FLOAT,
		INT,
		MAT2,
		MAT3,
		MAT4,
		IMAGE1D,
		IMAGE1D_ARRAY,
		IMAGE2D,
		IMAGE2D_ARRAY,
		IMAGE3D,
		TEXTURE1D,
		TEXTURE1D_ARRAY,
		TEXTURE2D,
		TEXTURE2D_ARRAY,
		TEXTURE3D,
		UINT,
		UVEC2,
		UVEC3,
		UVEC4,
		VEC2,
		VEC3,
		VEC4
	};

	using UniformLocation = int;

	struct Uniform
	{
		UniformLocation location = -1;
		UniformType type = UniformType::INT;
	};

	struct UniformTex : public Uniform
	{
		unsigned int bindingSlot = 0;
	};
};