#include <nex/mesh/MeshFactory.hpp>
#include "Mesh.hpp"
#include <nex/buffer/VertexBuffer.hpp>
#include <nex/buffer/IndexBuffer.hpp>
#include "VertexArray.hpp"
#include "VertexLayout.hpp"

using namespace std;

namespace nex
{
	std::unique_ptr<Mesh> MeshFactory::create(const MeshStore* store)
	{
		auto* skinnedVersion = dynamic_cast<const SkinnedMeshStore*>(store);
		if (skinnedVersion)
			return create(*skinnedVersion);
		return create(*store);
	}
	std::unique_ptr<Mesh> MeshFactory::create(const MeshStore& store)
	{
		auto mesh = std::make_unique<Mesh>();
		init(*mesh, store);
		return mesh;
	}

	std::unique_ptr<Mesh> MeshFactory::create(const SkinnedMeshStore& store)
	{
		auto mesh = std::make_unique<SkinnedMesh>();
		init(*mesh, store);
		mesh->setRigID(store.rigID);
		return mesh;
	}

	unique_ptr<Mesh> MeshFactory::create(const VertexPositionNormalTexTangent* vertices, uint32_t vertexCount, const uint32_t* indices, uint32_t indexCount, AABB boundingBox)
	{
		using Vertex = VertexPositionNormalTexTangent;


		auto vertexBuffer = std::make_unique<VertexBuffer>();

		vertexBuffer->resize(vertexCount * sizeof(Vertex), vertices, GpuBuffer::UsageHint::STATIC_DRAW);
		IndexBuffer indexBuffer(IndexElementType::BIT_32, indexCount, indices);

		VertexLayout layout;
		layout.push<glm::vec3>(1, vertexBuffer.get(), false, false); // position
		layout.push<glm::vec3>(1, vertexBuffer.get(), false, false); // normal
		layout.push<glm::vec2>(1, vertexBuffer.get(), false, false); // uv
		layout.push<glm::vec3>(1, vertexBuffer.get(), false, false); // tangent


		auto mesh = std::make_unique<Mesh>();
		mesh->addVertexDataBuffer(std::move(vertexBuffer));
		mesh->setBoundingBox(std::move(boundingBox));
		mesh->setIndexBuffer(std::move(indexBuffer));
		mesh->setLayout(std::move(layout));
		mesh->setTopology(Topology::TRIANGLES);
		mesh->setIsLoaded(true);

		return mesh;
	}

	unique_ptr<Mesh> MeshFactory::create(const VertexPositionNormalTex * vertices, uint32_t vertexCount, const uint32_t * indices, uint32_t indexCount,
		AABB boundingBox)
	{
		using Vertex = VertexPositionNormalTex;

		auto vertexBuffer = std::make_unique<VertexBuffer>();
		vertexBuffer->resize(vertexCount * sizeof(Vertex), vertices, GpuBuffer::UsageHint::STATIC_DRAW);
		IndexBuffer indexBuffer(IndexElementType::BIT_32, indexCount, indices);

		VertexLayout layout;
		layout.push<glm::vec3>(1, vertexBuffer.get(), false, false); // position
		layout.push<glm::vec3>(1, vertexBuffer.get(), false, false); // normal
		layout.push<glm::vec2>(1, vertexBuffer.get(), false, false); // uv


		auto mesh = std::make_unique<Mesh>();
		mesh->addVertexDataBuffer(std::move(vertexBuffer));
		mesh->setBoundingBox(std::move(boundingBox));
		mesh->setIndexBuffer(std::move(indexBuffer));
		mesh->setLayout(std::move(layout));
		mesh->setTopology(Topology::TRIANGLES);
		mesh->setIsLoaded(true);

		return mesh;
	}


	unique_ptr<Mesh> MeshFactory::createPosition(const VertexPosition* vertices, uint32_t vertexCount, const uint32_t* indices, uint32_t indexCount,
		AABB boundingBox)
	{
		using Vertex = VertexPosition;

		auto vertexBuffer = std::make_unique<VertexBuffer>();
		vertexBuffer->resize(vertexCount * sizeof(Vertex), vertices, GpuBuffer::UsageHint::STATIC_DRAW);
		IndexBuffer indexBuffer(IndexElementType::BIT_32, indexCount, indices);

		VertexLayout layout;
		layout.push<glm::vec3>(1, vertexBuffer.get(), false, false); // position

		auto mesh = std::make_unique<Mesh>();
		mesh->addVertexDataBuffer(std::move(vertexBuffer));
		mesh->setBoundingBox(std::move(boundingBox));
		mesh->setIndexBuffer(std::move(indexBuffer));
		mesh->setLayout(std::move(layout));
		mesh->setTopology(Topology::TRIANGLES);
		mesh->setIsLoaded(true);

		return mesh;
	}

	unique_ptr<Mesh> MeshFactory::createPositionUV(const VertexPositionTex* vertices, uint32_t vertexCount, const uint32_t* indices, uint32_t indexCount,
		AABB boundingBox)
	{
		using Vertex = VertexPositionTex;

		auto vertexBuffer = std::make_unique<VertexBuffer>();
		vertexBuffer->resize(vertexCount * sizeof(Vertex), vertices, GpuBuffer::UsageHint::STATIC_DRAW);
		IndexBuffer indexBuffer(IndexElementType::BIT_32, indexCount, indices);

		VertexLayout layout;
		layout.push<glm::vec3>(1, vertexBuffer.get(), false, false); // position
		layout.push<glm::vec2>(1, vertexBuffer.get(), false, false); // uv

		auto mesh = std::make_unique<Mesh>();
		mesh->addVertexDataBuffer(std::move(vertexBuffer));
		mesh->setBoundingBox(std::move(boundingBox));
		mesh->setIndexBuffer(std::move(indexBuffer));
		mesh->setLayout(std::move(layout));
		mesh->setTopology(Topology::TRIANGLES);
		mesh->setIsLoaded(true);

		return mesh;
	}
	void MeshFactory::init(Mesh& mesh, const MeshStore& store)
	{
		auto vertexBuffer = std::make_unique<VertexBuffer>();
		vertexBuffer->resize(store.vertices.size(), store.vertices.data(), GpuBuffer::UsageHint::STATIC_DRAW);
		IndexBuffer indexBuffer(store.indexType, store.indices.size() / getIndexElementTypeByteSize(store.indexType),
			store.indices.data());

		indexBuffer.unbind();

		mesh.setBoundingBox(store.boundingBox);
		mesh.setIndexBuffer(std::move(indexBuffer));

		auto& layout = mesh.getLayout();
		layout = store.layout;
		auto& attributes = layout.getAttributes();
		for (auto& attribute : attributes)
		{
			attribute.buffer = vertexBuffer.get();
		}

		mesh.addVertexDataBuffer(std::move(vertexBuffer));
		mesh.setTopology(Topology::TRIANGLES);
		mesh.setIsLoaded();
	}
}