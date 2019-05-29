#pragma once

#include <glm/glm.hpp>
#include <istream>
#include <ostream>

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

	inline constexpr size_t getIndexElementTypeByteSize(IndexElementType type)
	{
		static size_t table[] = 
		{
			2,
			4
		};

		static_assert(sizeof(table) / sizeof(size_t) == ((size_t)IndexElementType::LAST) + 1);

		return table[(size_t)type];
	}


	inline std::istream & operator>> (std::istream & in, nex::IndexElementType & type)
	{
		in.read((char*)&type, sizeof(nex::IndexElementType));
		return in;
	}

	inline std::ostream& operator<<(std::ostream& out, const nex::IndexElementType& type)
	{
		out.write((char*)&type, sizeof(nex::IndexElementType));
		return out;
	}


	inline std::istream & operator>> (std::istream & in, nex::Topology & topology)
	{
		in.read((char*)&topology, sizeof(nex::Topology));

		return in;
	}

	inline std::ostream& operator<<(std::ostream& out, const nex::Topology& topology)
	{
		out.write((char*)&topology, sizeof(nex::Topology));
		return out;
	}
}