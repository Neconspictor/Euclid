#pragma once
#include "nex/mesh/IndexBuffer.hpp"
#include "nex/mesh/SubMesh.hpp"
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


	GLuint translate(Topology topology);
	GLuint translate(IndexElementType indexType);
}