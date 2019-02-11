#pragma once
#include "nex/mesh/IndexBuffer.hpp"
#include "nex/mesh/SubMesh.hpp"
#include <nex/RenderBackend.hpp>
#include <nex/opengl/opengl.hpp>

namespace nex {

	enum TopologyGL
	{
		LINES = GL_LINES,
		LINES_ADJACENCY = GL_LINES_ADJACENCY,
		LINE_LOOP = GL_LINE_LOOP,
		LINE_STRIP = GL_LINE_STRIP,
		LINE_STRIP_ADJACENCY = GL_LINE_STRIP_ADJACENCY,
		PATCHES = GL_PATCHES,
		POINTS = GL_POINTS,
		TRIANGLES = GL_TRIANGLES,
		TRIANGLES_ADJACENCY = GL_TRIANGLES_ADJACENCY,
		TRIANGLE_FAN = GL_TRIANGLE_FAN,
		TRIANGLE_STRIP = GL_TRIANGLE_STRIP,
		TRIANGLE_STRIP_ADJACENCY = GL_TRIANGLE_STRIP_ADJACENCY,
	};

	enum IndexElementTypeGL {
		BIT_16 = GL_UNSIGNED_SHORT,
		BIT_32 = GL_UNSIGNED_INT,
	};

	enum PolygonSideGL
	{
		BACK = GL_BACK,
		FRONT = GL_FRONT,
		FRONT_BACK = GL_FRONT_AND_BACK,
	};

	enum FillTypeGL
	{
		FILL = GL_FILL,
		LINE = GL_LINE,
		POINT = GL_POINT,
	};

	enum BlendFuncGL
	{
		ZERO = GL_ZERO,
		ONE = GL_ONE,

		SOURCE_COLOR = GL_SRC_COLOR,
		ONE_MINUS_SOURCE_COLOR = GL_ONE_MINUS_SRC_COLOR,
		DESTINATION_COLOR = GL_DST_COLOR,
		ONE_MINUS_DESTINATION_COLOR = GL_ONE_MINUS_DST_COLOR,

		SOURCE_ALPHA = GL_SRC_ALPHA,
		ONE_MINUS_SOURCE_ALPHA = GL_ONE_MINUS_SRC_ALPHA,
		DESTINATION_ALPHA = GL_DST_ALPHA,
		ONE_MINUS_DESTINATION_ALPHA = GL_ONE_MINUS_DST_ALPHA,

		CONSTANT_COLOR = GL_CONSTANT_COLOR,
		ONE_MINUS_CONSTANT_COLOR = GL_ONE_MINUS_CONSTANT_COLOR,
		CONSTANT_ALPHA = GL_CONSTANT_ALPHA,
		ONE_MINUS_CONSTANT_ALPHA = GL_ONE_MINUS_CONSTANT_ALPHA,
	};

	enum BlendOperationGL
	{
		ADD = GL_FUNC_ADD, // source + destination
		SUBTRACT = GL_FUNC_SUBTRACT, // source - destination
		REV_SUBTRACT = GL_FUNC_REVERSE_SUBTRACT, // destination - source
		MIN = GL_MIN, // min(source, destination)
		MAX = GL_MAX, // max(source, destination)
	};

	struct RenderTargetBlendDescGL
	{
		GLuint enableBlend;
		unsigned colorAttachIndex;
		BlendFuncGL sourceRGB;
		BlendFuncGL destRGB;
		BlendOperationGL operationRGB;
		BlendFuncGL sourceAlpha;
		BlendFuncGL destAlpha;
		BlendOperationGL operationAlpha;
		//unsigned char renderTargetWriteMask; // not supported by opengl

		RenderTargetBlendDescGL(const RenderTargetBlendDesc& desc);
	};

	GLuint translate(IndexElementType indexType);
	GLuint translate(PolygonSide side);
	GLuint translate(FillMode type);
	GLuint translate(Topology topology);

	BlendFuncGL translate(BlendFunc func);
	BlendOperationGL translate(BlendOperation op);
}