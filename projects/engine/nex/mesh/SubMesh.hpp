#pragma once

#include <glm/glm.hpp>
#include "VertexArray.hpp"
#include "IndexBuffer.hpp"

namespace nex
{
	class Material;
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

		SubMesh(VertexArray vertexArray, IndexBuffer indexBuffer, Material* material = nullptr);
		SubMesh();

		SubMesh(SubMesh&& other) noexcept = default;
		SubMesh& operator=(SubMesh&& o) noexcept = default;

		SubMesh(const SubMesh& o) = delete;
		SubMesh& operator=(const SubMesh& o) = delete;

		virtual ~SubMesh() = default;

		IndexBuffer* getIndexBuffer();
		Material* getMaterial() const;
		VertexArray* getVertexArray();

		void setIndexBuffer(IndexBuffer buffer);
		void setMaterial(Material* material);
		void setVertexArray(VertexArray vertexArray);

	protected:
		VertexArray mVertexArray;
		IndexBuffer mIndexBuffer;

		Material* mMaterial;
	};
}
