#include <nex/mesh/MeshFactory.hpp>
#include "Mesh.hpp"
#include <nex/buffer/VertexBuffer.hpp>
#include <nex/buffer/IndexBuffer.hpp>
#include "VertexArray.hpp"
#include "VertexLayout.hpp"

using namespace std;

namespace nex
{
	std::unique_ptr<Mesh> MeshFactory::create(const MeshStore& store)
	{
		VertexBuffer vertexBuffer;
		vertexBuffer.bind();
		vertexBuffer.resize(store.vertices.size(), store.vertices.data(), GpuBuffer::UsageHint::STATIC_DRAW);
		IndexBuffer indexBuffer(store.indexType, store.indices.size() / getIndexElementTypeByteSize(store.indexType), 
			store.indices.data());

		indexBuffer.unbind();

		auto mesh = std::make_unique<Mesh>();
		mesh->init(std::move(vertexBuffer), store.layout, std::move(indexBuffer), store.boundingBox, Topology::TRIANGLES);
		return mesh;
	}

	unique_ptr<Mesh> MeshFactory::create(const VertexPositionNormalTexTangent* vertices, uint32_t vertexCount, const uint32_t* indices, uint32_t indexCount, AABB boundingBox)
	{
		using Vertex = VertexPositionNormalTexTangent;


		VertexBuffer vertexBuffer;
		vertexBuffer.bind();
		vertexBuffer.resize(vertexCount * sizeof(Vertex), vertices, GpuBuffer::UsageHint::STATIC_DRAW);
		IndexBuffer indexBuffer(IndexElementType::BIT_32, indexCount, indices);
		//indexBuffer.bind();

		VertexLayout layout;
		layout.push<glm::vec3>(1); // position
		layout.push<glm::vec3>(1); // normal
		layout.push<glm::vec2>(1); // uv
		layout.push<glm::vec3>(1); // tangent
		layout.push<glm::vec3>(1); // bitangent

		//indexBuffer.unbind();

		auto mesh = std::make_unique<Mesh>();
		mesh->init(std::move(vertexBuffer), std::move(layout), std::move(indexBuffer), std::move(boundingBox), Topology::TRIANGLES);

		return mesh;
	}

	unique_ptr<Mesh> MeshFactory::create(const VertexPositionNormalTex * vertices, uint32_t vertexCount, const uint32_t * indices, uint32_t indexCount,
		AABB boundingBox)
	{
		using Vertex = VertexPositionNormalTex;

		VertexBuffer vertexBuffer;
		vertexBuffer.resize(vertexCount * sizeof(Vertex), vertices, GpuBuffer::UsageHint::STATIC_DRAW);
		IndexBuffer indexBuffer(IndexElementType::BIT_32, indexCount, indices);
		indexBuffer.unbind();

		VertexLayout layout;
		layout.push<glm::vec3>(1); // position
		layout.push<glm::vec3>(1); // normal
		layout.push<glm::vec2>(1); // uv

		auto mesh = std::make_unique<Mesh>();
		mesh->init(std::move(vertexBuffer), std::move(layout), std::move(indexBuffer), std::move(boundingBox), Topology::TRIANGLES);

		return mesh;
	}


	unique_ptr<Mesh> MeshFactory::createPosition(const VertexPosition* vertices, uint32_t vertexCount, const uint32_t* indices, uint32_t indexCount,
		AABB boundingBox)
	{
		using Vertex = VertexPosition;

		VertexBuffer vertexBuffer;
		vertexBuffer.resize(vertexCount * sizeof(Vertex), vertices, GpuBuffer::UsageHint::STATIC_DRAW);
		IndexBuffer indexBuffer(IndexElementType::BIT_32, indexCount, indices);
		indexBuffer.unbind();

		VertexLayout layout;
		layout.push<glm::vec3>(1); // position

		auto mesh = std::make_unique<Mesh>();
		mesh->init(std::move(vertexBuffer), std::move(layout), std::move(indexBuffer), std::move(boundingBox), Topology::TRIANGLES);

		return mesh;
	}

	unique_ptr<Mesh> MeshFactory::createPositionUV(const VertexPositionTex* vertices, uint32_t vertexCount, const uint32_t* indices, uint32_t indexCount,
		AABB boundingBox)
	{
		using Vertex = VertexPositionTex;

		VertexBuffer vertexBuffer;
		vertexBuffer.resize(vertexCount * sizeof(Vertex), vertices, GpuBuffer::UsageHint::STATIC_DRAW);
		IndexBuffer indexBuffer(IndexElementType::BIT_32, indexCount, indices);
		indexBuffer.unbind();

		VertexLayout layout;
		layout.push<glm::vec3>(1); // position
		layout.push<glm::vec2>(1); // uv

		auto mesh = std::make_unique<Mesh>();
		mesh->init(std::move(vertexBuffer), std::move(layout), std::move(indexBuffer), std::move(boundingBox), Topology::TRIANGLES);

		return mesh;
	}
}