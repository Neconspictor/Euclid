#pragma once

#include <glm/glm.hpp>

namespace nex
{
	struct VertexPositionNormalTexTangent {
		glm::vec3 position;
		glm::vec3 normal;
		glm::vec2 texCoords;
		glm::vec3 tangent;
		glm::vec3 bitangent;
	};

	struct VertexPositionNormalTex {
		glm::vec3 position;
		glm::vec3 normal;
		glm::vec2 texCoords;
	};

	struct VertexPosition
	{
		glm::vec3 position;
	};

	struct VertexPositionTex
	{
		glm::vec3 position;
		glm::vec2 texCoords;
	};

	enum class Topology
	{
		LINES, FIRST = LINES,
		LINES_ADJACENCY,
		LINE_LOOP,
		LINE_STRIP,
		LINE_STRIP_ADJACENCY,
		PATCHES,
		POINTS,
		TRIANGLES,
		TRIANGLES_ADJACENCY,
		TRIANGLE_FAN,
		TRIANGLE_STRIP,
		TRIANGLE_STRIP_ADJACENCY, LAST = TRIANGLE_STRIP_ADJACENCY,
	};

	enum class IndexElementType {
		BIT_16 = 0, FIRST = BIT_16,
		BIT_32 = 1, LAST = BIT_32,
	};
}