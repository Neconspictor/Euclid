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

		/**
		 * The default mesh generation method.
		 * Creates a gl mesh with position and normal data, and uv coordinates.
		 */
		static std::unique_ptr<Mesh> create(const VertexPositionNormalTexTangent* vertices, size_t vertexCount,
			const unsigned* indices, size_t indexCount, AABB boundingBox);

		static std::unique_ptr<Mesh> create(const VertexPositionNormalTex* vertices, size_t vertexCount,
			const unsigned* indices, size_t indexCount, AABB boundingBox);

		/**
		* Creates a mesh with position data.
		*/
		static std::unique_ptr<Mesh> createPosition(const VertexPosition* vertices, size_t vertexCount,
			const unsigned* indices, size_t indexCount, AABB boundingBox);

		/**
		* Creates a mesh with position data and uv coordinates.
		*/
		static std::unique_ptr<Mesh> createPositionUV(const VertexPositionTex* vertices, size_t vertexCount,
			const unsigned* indices, size_t indexCount, AABB boundingBox);

	private:

		static std::unique_ptr<Mesh> create(const MeshStore& store);
		static std::unique_ptr<Mesh> create(const SkinnedMeshStore& store);
		static void init(Mesh& mesh, const MeshStore& store);
	};
}