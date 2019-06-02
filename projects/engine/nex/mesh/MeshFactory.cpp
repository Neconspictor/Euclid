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
		indexBuffer.bind();

		VertexArray vertexArray;
		vertexArray.bind();
		vertexArray.useBuffer(vertexBuffer, store.layout);

		vertexArray.unbind();
		indexBuffer.unbind();

		return std::make_unique<Mesh>(std::move(vertexArray), std::move(vertexBuffer), std::move(indexBuffer), store.boundingBox);
	}

	unique_ptr<Mesh> MeshFactory::create(const VertexPositionNormalTexTangent* vertices, uint32_t vertexCount, const uint32_t* indices, uint32_t indexCount, AABB boundingBox)
	{
		using Vertex = VertexPositionNormalTexTangent;


		VertexBuffer vertexBuffer;
		vertexBuffer.bind();
		vertexBuffer.fill(vertices, vertexCount * sizeof(Vertex));
		IndexBuffer indexBuffer(indices, indexCount, IndexElementType::BIT_32);
		indexBuffer.bind();

		VertexLayout layout;
		layout.push<glm::vec3>(1); // position
		layout.push<glm::vec3>(1); // normal
		layout.push<glm::vec2>(1); // uv
		layout.push<glm::vec3>(1); // tangent
		layout.push<glm::vec3>(1); // bitangent

		VertexArray vertexArray;
		vertexArray.bind();
		vertexArray.useBuffer(vertexBuffer, layout);

		vertexArray.unbind();
		indexBuffer.unbind();

		return std::make_unique<Mesh>(std::move(vertexArray), std::move(vertexBuffer), std::move(indexBuffer), std::move(boundingBox));
	}

	unique_ptr<Mesh> MeshFactory::create(const VertexPositionNormalTex * vertices, uint32_t vertexCount, const uint32_t * indices, uint32_t indexCount,
		AABB boundingBox)
	{
		using Vertex = VertexPositionNormalTex;

		VertexBuffer vertexBuffer;
		vertexBuffer.fill(vertices, vertexCount * sizeof(Vertex));
		IndexBuffer indexBuffer(indices, indexCount, IndexElementType::BIT_32);

		VertexLayout layout;
		layout.push<glm::vec3>(1); // position
		layout.push<glm::vec3>(1); // normal
		layout.push<glm::vec2>(1); // uv

		VertexArray vertexArray;
		vertexArray.bind();
		vertexArray.useBuffer(vertexBuffer, layout);

		vertexArray.unbind();
		indexBuffer.unbind();

		return std::make_unique<Mesh>(std::move(vertexArray), std::move(vertexBuffer), std::move(indexBuffer), std::move(boundingBox));
	}


	unique_ptr<Mesh> MeshFactory::createPosition(const VertexPosition* vertices, uint32_t vertexCount, const uint32_t* indices, uint32_t indexCount,
		AABB boundingBox)
	{
		using Vertex = VertexPosition;

		VertexBuffer vertexBuffer;
		vertexBuffer.fill(vertices, vertexCount * sizeof(Vertex));
		IndexBuffer indexBuffer(indices, indexCount, IndexElementType::BIT_32);

		VertexLayout layout;
		layout.push<glm::vec3>(1); // position

		VertexArray vertexArray;
		vertexArray.bind();
		vertexArray.useBuffer(vertexBuffer, layout);

		vertexArray.unbind();
		indexBuffer.unbind();

		return std::make_unique<Mesh>(std::move(vertexArray), std::move(vertexBuffer), std::move(indexBuffer), std::move(boundingBox));
	}

	unique_ptr<Mesh> MeshFactory::createPositionUV(const VertexPositionTex* vertices, uint32_t vertexCount, const uint32_t* indices, uint32_t indexCount,
		AABB boundingBox)
	{
		using Vertex = VertexPositionTex;

		VertexBuffer vertexBuffer;
		vertexBuffer.fill(vertices, vertexCount * sizeof(Vertex));
		IndexBuffer indexBuffer(indices, indexCount, IndexElementType::BIT_32);

		VertexLayout layout;
		layout.push<glm::vec3>(1); // position
		layout.push<glm::vec2>(1); // uv

		VertexArray vertexArray;
		vertexArray.bind();
		vertexArray.useBuffer(vertexBuffer, layout);

		vertexArray.unbind();
		indexBuffer.unbind();

		return std::make_unique<Mesh>(std::move(vertexArray), std::move(vertexBuffer), std::move(indexBuffer),
			std::move(boundingBox));
	}
}