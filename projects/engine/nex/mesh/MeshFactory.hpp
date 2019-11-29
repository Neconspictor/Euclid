#pragma once
#include <nex/mesh/Mesh.hpp>
#include <nex/mesh/MeshStore.hpp>

namespace nex
{

	class MeshFactory
	{
	public:

		template<typename type>
		struct SimpleArray
		{
			const type* content;
			uint32_t size;
		};

		/**
		 * Creates an initialized (but not finalized!) mesh from a mesh store.
		 * It checks inheritance and creates a suitable mesh subclass.
		 */
		static std::unique_ptr<Mesh> create(const MeshStore* store);

		static std::unique_ptr<Mesh> create(const void* vertices, size_t verticesSize, VertexLayout layout);


		/**
		 * The default mesh generation method.
		 * Creates a gl mesh with position and normal data, and uv coordinates.
		 */
		static std::unique_ptr<Mesh> create(const VertexPositionNormalTexTangent* vertices, uint32_t vertexCount,
			const uint32_t* indices, uint32_t indexCount, AABB boundingBox);

		static std::unique_ptr<Mesh> create(const VertexPositionNormalTex* vertices, uint32_t vertexCount,
			const uint32_t* indices, uint32_t indexCount, AABB boundingBox);

		/**
		* Creates a mesh with position data.
		*/
		static std::unique_ptr<Mesh> createPosition(const VertexPosition* vertices, uint32_t vertexCount,
			const uint32_t* indices, uint32_t indexCount, AABB boundingBox);

		/**
		* Creates a mesh with position data and uv coordinates.
		*/
		static std::unique_ptr<Mesh> createPositionUV(const VertexPositionTex* vertices, uint32_t vertexCount,
			const uint32_t* indices, uint32_t indexCount, AABB boundingBox);

	private:

		static std::unique_ptr<Mesh> create(const MeshStore& store);
		static std::unique_ptr<Mesh> create(const SkinnedMeshStore& store);
		static void init(Mesh& mesh, const MeshStore& store);
	};
}