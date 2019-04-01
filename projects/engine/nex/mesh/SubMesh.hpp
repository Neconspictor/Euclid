#pragma once

#include <glm/glm.hpp>
#include "VertexArray.hpp"
#include "VertexBuffer.hpp"
#include "IndexBuffer.hpp"
#include <nex/material/Material.hpp>

namespace nex
{
	class MeshFactory;

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


	/**
	 * Represents a 3d mesh consisting of vertices and a list of indices describing
	 * a stream of three sided polygons. A vertex describes the position of a 3d point
	 * and can have additional information like a normal and texture uv coordinates.
	 * The so-called vertex slice describes of how many (float) data elements a vertex
	 * consists of. Theoretically, a vertex isn't bound to floating point units, but this
	 * implementation narrows it to floats for ease of use.
	 */
	class SubMesh
	{
	public:
		using Vertex = VertexPositionNormalTexTangent;

		SubMesh(VertexArray vertexArray, VertexBuffer vertexBuffer, IndexBuffer indexBuffer, Topology topology = Topology::TRIANGLES, Material* material = nullptr);
		SubMesh();

		SubMesh(SubMesh&& other) noexcept = default;
		SubMesh& operator=(SubMesh&& o) noexcept = default;

		SubMesh(const SubMesh& o) = delete;
		SubMesh& operator=(const SubMesh& o) = delete;

		virtual ~SubMesh() = default;

		IndexBuffer* getIndexBuffer();
		Material* getMaterial() const;
		Topology getTopology() const;
		VertexArray* getVertexArray();
		VertexBuffer* getVertexBuffer();

		void setIndexBuffer(IndexBuffer buffer);
		void setMaterial(Material* material);
		void setTopology(Topology topology);
		void setVertexArray(VertexArray vertexArray);
		void setVertexBuffer(VertexBuffer buffer);
		

	protected:
		VertexArray mVertexArray;
		IndexBuffer mIndexBuffer;
		VertexBuffer mVertexBuffer;

		Material* mMaterial;

		Topology mTopology;
	};
}