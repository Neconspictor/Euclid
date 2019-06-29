#include <nex/mesh/MeshFactory.hpp>
#include "Mesh.hpp"
#include "VertexBuffer.hpp"
#include "IndexBuffer.hpp"
#include "VertexArray.hpp"
#include "VertexLayout.hpp"

using namespace std;

namespace nex
{
	std::unique_ptr<Mesh> MeshFactory::create(const MeshStore& store)
	{
		VertexBuffer vertexBuffer;
		vertexBuffer.bind();
		vertexBuffer.fill(store.vertices.data(), store.vertices.size());
		IndexBuffer indexBuffer(store.indices.data(), 
			store.indices.size() / getIndexElementTypeByteSize(store.indexType), 
			store.indexType);
		indexBuffer.unbind();

		return std::make_unique<Mesh>(std::move(vertexBuffer), store.layout, std::move(indexBuffer), store.boundingBox, Topology::TRIANGLES, false);
	}

	unique_ptr<Mesh> MeshFactory::create(const VertexPositionNormalTexTangent* vertices, uint32_t vertexCount, const uint32_t* indices, uint32_t indexCount, AABB boundingBox)
	{
		using Vertex = VertexPositionNormalTexTangent;


		VertexBuffer vertexBuffer;
		vertexBuffer.bind();
		vertexBuffer.fill(vertices, vertexCount * sizeof(Vertex));
		IndexBuffer indexBuffer(indices, indexCount, IndexElementType::BIT_32);
		//indexBuffer.bind();

		VertexLayout layout;
		layout.push<glm::vec3>(1); // position
		layout.push<glm::vec3>(1); // normal
		layout.push<glm::vec2>(1); // uv
		layout.push<glm::vec3>(1); // tangent
		layout.push<glm::vec3>(1); // bitangent

		//indexBuffer.unbind();

		return std::make_unique<Mesh>(std::move(vertexBuffer), std::move(layout), std::move(indexBuffer), std::move(boundingBox), Topology::TRIANGLES, false);
	}

	unique_ptr<Mesh> MeshFactory::create(const VertexPositionNormalTex * vertices, uint32_t vertexCount, const uint32_t * indices, uint32_t indexCount,
		AABB boundingBox)
	{
		using Vertex = VertexPositionNormalTex;

		VertexBuffer vertexBuffer;
		vertexBuffer.fill(vertices, vertexCount * sizeof(Vertex));
		IndexBuffer indexBuffer(indices, indexCount, IndexElementType::BIT_32);
		indexBuffer.unbind();

		VertexLayout layout;
		layout.push<glm::vec3>(1); // position
		layout.push<glm::vec3>(1); // normal
		layout.push<glm::vec2>(1); // uv

		return std::make_unique<Mesh>(std::move(vertexBuffer), std::move(layout), std::move(indexBuffer), std::move(boundingBox), Topology::TRIANGLES, false);
	}


	unique_ptr<Mesh> MeshFactory::createPosition(const VertexPosition* vertices, uint32_t vertexCount, const uint32_t* indices, uint32_t indexCount,
		AABB boundingBox)
	{
		using Vertex = VertexPosition;

		VertexBuffer vertexBuffer;
		vertexBuffer.fill(vertices, vertexCount * sizeof(Vertex));
		IndexBuffer indexBuffer(indices, indexCount, IndexElementType::BIT_32);
		indexBuffer.unbind();

		VertexLayout layout;
		layout.push<glm::vec3>(1); // position

		return std::make_unique<Mesh>(std::move(vertexBuffer), std::move(layout), std::move(indexBuffer), std::move(boundingBox), Topology::TRIANGLES, false);
	}

	unique_ptr<Mesh> MeshFactory::createPositionUV(const VertexPositionTex* vertices, uint32_t vertexCount, const uint32_t* indices, uint32_t indexCount,
		AABB boundingBox)
	{
		using Vertex = VertexPositionTex;

		VertexBuffer vertexBuffer;
		vertexBuffer.fill(vertices, vertexCount * sizeof(Vertex));
		IndexBuffer indexBuffer(indices, indexCount, IndexElementType::BIT_32);
		indexBuffer.unbind();

		VertexLayout layout;
		layout.push<glm::vec3>(1); // position
		layout.push<glm::vec2>(1); // uv

		return std::make_unique<Mesh>(std::move(vertexBuffer), std::move(layout), std::move(indexBuffer), std::move(boundingBox), Topology::TRIANGLES, false);
	}
}