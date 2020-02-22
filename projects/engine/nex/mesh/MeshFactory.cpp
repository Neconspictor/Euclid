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
		std::unique_ptr<Mesh> mesh;

		if (store.isSkinned) {
			mesh = std::make_unique<SkinnedMesh>();
			SkinnedMesh* meshPtr= (SkinnedMesh*)mesh.get();
			meshPtr->setRigID(store.rigID);
		}
		else {
			mesh = std::make_unique<Mesh>();
		}

		init(*mesh, store);
		return mesh;
	}

	unique_ptr<Mesh> MeshFactory::create(const VertexPositionNormalTexTangent* vertices, 
		size_t vertexCount, 
		const unsigned* indices,
		size_t indexCount, 
		AABB boundingBox)
	{
		using Vertex = VertexPositionNormalTexTangent;


		auto vertexBuffer = std::make_unique<VertexBuffer>();

		vertexBuffer->resize(vertexCount * sizeof(Vertex), vertices, GpuBuffer::UsageHint::STATIC_DRAW);
		IndexBuffer indexBuffer(IndexElementType::BIT_32, indexCount, indices);

		VertexLayout layout;
		layout.push<glm::vec3>(1, vertexBuffer.get(), false, false, true); // position
		layout.push<glm::vec3>(1, vertexBuffer.get(), false, false, true); // normal
		layout.push<glm::vec2>(1, vertexBuffer.get(), false, false, true); // uv
		layout.push<glm::vec3>(1, vertexBuffer.get(), false, false, true); // tangent


		auto mesh = std::make_unique<Mesh>();
		mesh->getVertexArray().setLayout(layout);
		mesh->addVertexDataBuffer(std::move(vertexBuffer));
		mesh->setBoundingBox(std::move(boundingBox));
		mesh->setIndexBuffer(std::move(indexBuffer));
		mesh->setTopology(Topology::TRIANGLES);
		mesh->setUseIndexBuffer(true);
		mesh->setVertexCount(vertexCount);

		return mesh;
	}

	unique_ptr<Mesh> MeshFactory::create(const VertexPositionNormalTex * vertices, 
		size_t vertexCount, 
		const unsigned* indices,
		size_t indexCount,
		AABB boundingBox)
	{
		using Vertex = VertexPositionNormalTex;

		auto vertexBuffer = std::make_unique<VertexBuffer>();
		vertexBuffer->resize(vertexCount * sizeof(Vertex), vertices, GpuBuffer::UsageHint::STATIC_DRAW);
		IndexBuffer indexBuffer(IndexElementType::BIT_32, indexCount, indices);

		VertexLayout layout;
		layout.push<glm::vec3>(1, vertexBuffer.get(), false, false, true); // position
		layout.push<glm::vec3>(1, vertexBuffer.get(), false, false, true); // normal
		layout.push<glm::vec2>(1, vertexBuffer.get(), false, false, true); // uv


		auto mesh = std::make_unique<Mesh>();
		mesh->getVertexArray().setLayout(layout);
		mesh->addVertexDataBuffer(std::move(vertexBuffer));
		mesh->setBoundingBox(std::move(boundingBox));
		mesh->setIndexBuffer(std::move(indexBuffer));
		mesh->setTopology(Topology::TRIANGLES);
		mesh->setUseIndexBuffer(true);
		mesh->setVertexCount(vertexCount);

		return mesh;
	}


	unique_ptr<Mesh> MeshFactory::createPosition(const VertexPosition* vertices, 
		size_t vertexCount, 
		const unsigned* indices,
		size_t indexCount,
		AABB boundingBox)
	{
		using Vertex = VertexPosition;

		auto vertexBuffer = std::make_unique<VertexBuffer>();
		vertexBuffer->resize(vertexCount * sizeof(Vertex), vertices, GpuBuffer::UsageHint::STATIC_DRAW);
		IndexBuffer indexBuffer(IndexElementType::BIT_32, indexCount, indices);

		VertexLayout layout;
		layout.push<glm::vec3>(1, vertexBuffer.get(), false, false, true); // position

		auto mesh = std::make_unique<Mesh>();
		mesh->getVertexArray().setLayout(layout);
		mesh->addVertexDataBuffer(std::move(vertexBuffer));
		mesh->setBoundingBox(std::move(boundingBox));
		mesh->setIndexBuffer(std::move(indexBuffer));
		mesh->setTopology(Topology::TRIANGLES);
		mesh->setUseIndexBuffer(true);
		mesh->setVertexCount(vertexCount);

		return mesh;
	}

	unique_ptr<Mesh> MeshFactory::createPositionUV(const VertexPositionTex* vertices, 
		size_t vertexCount, 
		const unsigned* indices,
		size_t indexCount,
		AABB boundingBox)
	{
		using Vertex = VertexPositionTex;

		auto vertexBuffer = std::make_unique<VertexBuffer>();
		vertexBuffer->resize(vertexCount * sizeof(Vertex), vertices, GpuBuffer::UsageHint::STATIC_DRAW);
		IndexBuffer indexBuffer(IndexElementType::BIT_32, indexCount, indices);

		VertexLayout layout;
		layout.push<glm::vec3>(1, vertexBuffer.get(), false, false, true); // position
		layout.push<glm::vec2>(1, vertexBuffer.get(), false, false, true); // uv

		auto mesh = std::make_unique<Mesh>();
		mesh->getVertexArray().setLayout(layout);
		mesh->addVertexDataBuffer(std::move(vertexBuffer));
		mesh->setBoundingBox(std::move(boundingBox));
		mesh->setIndexBuffer(std::move(indexBuffer));
		mesh->setTopology(Topology::TRIANGLES);
		mesh->setUseIndexBuffer(true);
		mesh->setVertexCount(vertexCount);

		return mesh;
	}
	

	void MeshFactory::init(Mesh& mesh, const MeshStore& store)
	{
		auto& layout = mesh.getVertexArray().getLayout();
		layout = store.layout;
		auto& map = layout.getBufferLayoutMap();

		for (const auto& it : store.verticesMap) {
			const auto& vertices = it.second;
			const auto* invalidGpuBuffer = it.first;

			auto vertexBuffer = std::make_unique<VertexBuffer>();
			vertexBuffer->resize(vertices.size(), vertices.data(), GpuBuffer::UsageHint::STATIC_DRAW);

			auto it = map.extract(map.find(invalidGpuBuffer));
			it.key() = vertexBuffer.get();
			map.insert(std::move(it));

			mesh.addVertexDataBuffer(std::move(vertexBuffer));
		}

		if (store.useIndexBuffer) {
			IndexBuffer indexBuffer(store.indexType, store.indices.size() / getIndexElementTypeByteSize(store.indexType),
				store.indices.data());
			indexBuffer.unbind();
			mesh.setIndexBuffer(std::move(indexBuffer));
		}


		mesh.setBoundingBox(store.boundingBox);
		mesh.setTopology(store.topology);
		mesh.setArrayOffset(store.arrayOffset);
		mesh.setUseIndexBuffer(store.useIndexBuffer);
		mesh.setVertexCount(store.vertexCount);
	}
}