#pragma once

#include <glm/glm.hpp>
#include <nex/opengl/material/Material.hpp>
#include <memory>
#include <nex/opengl/mesh/VertexArray.hpp>
#include <nex/opengl/mesh/IndexBuffer.hpp>

namespace nex
{
	class MeshFactoryGL;

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
	class MeshGL
	{
	public:
		using Vertex = VertexPositionNormalTexTangent;

		MeshGL(VertexArray vertexArray, IndexBuffer indexBuffer, Material* material = nullptr);
		MeshGL();

		MeshGL(MeshGL&& other) noexcept = default;
		MeshGL& operator=(MeshGL&& o) noexcept = default;

		MeshGL(const MeshGL& o) = delete;
		MeshGL& operator=(const MeshGL& o) = delete;

		virtual ~MeshGL() = default;

		IndexBuffer* getIndexBuffer();
		Material* getMaterial() const;
		VertexArray* getVertexArray();

		void setIndexBuffer(IndexBuffer buffer);
		void setMaterial(Material* material);
		void setVertexArray(VertexArray vertexArray);

	protected:
		friend MeshFactoryGL; // allow factory for easier access!

		VertexArray mVertexArray;
		IndexBuffer mIndexBuffer;

		Material* mMaterial;
	};
}