#pragma once
#include <nex/mesh/SubMesh.hpp>

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
		 * The default mesh generation method.
		 * Creates a gl mesh with position and normal data, and uv coordinates.
		 */
		static std::unique_ptr<Mesh> create(const VertexPositionNormalTexTangent* vertices, uint32_t vertexCount,
			const uint32_t* indices, uint32_t indexCount);

		static std::unique_ptr<Mesh> create(const VertexPositionNormalTex* vertices, uint32_t vertexCount,
			const uint32_t* indices, uint32_t indexCount);

		/**
		* Creates a gl mesh with position data.
		*/
		static std::unique_ptr<Mesh> createPosition(const VertexPosition* vertices, uint32_t vertexCount,
			const uint32_t* indices, uint32_t indexCount);

		/**
		* Creates a gl mesh with position data and uv coordinates.
		*/
		static std::unique_ptr<Mesh> createPositionUV(const VertexPositionTex* vertices, uint32_t vertexCount,
			const uint32_t* indices, uint32_t indexCount);
	};
}