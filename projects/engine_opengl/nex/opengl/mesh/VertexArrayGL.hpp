#pragma once

#include <nex/mesh/VertexArray.hpp>
#include <nex/opengl/opengl.hpp>

namespace nex
{

	enum class LayoutPrimitive;

	enum LayoutTypeGL
	{
		UNSIGNED_INT = GL_UNSIGNED_INT,
		FLOAT = GL_FLOAT,
		UNSIGNED_BYTE = GL_UNSIGNED_BYTE,
		UNSIGNED_SHORT = GL_UNSIGNED_SHORT,
	};


	LayoutTypeGL translate(LayoutPrimitive type);

	bool isIntegerType(LayoutTypeGL type);
	bool isFloatType(LayoutTypeGL type);
	bool isDoubleType(LayoutTypeGL type);
}