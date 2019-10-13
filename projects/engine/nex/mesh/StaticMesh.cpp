#include <nex/mesh/StaticMesh.hpp>
#include <nex/Scene.hpp>
#include <nex/material/Material.hpp>
#include "MeshFactory.hpp"
#include "nex/material/AbstractMaterialLoader.hpp"
#include <nex/math/Math.hpp>

namespace nex
{
	StaticMeshContainer::~StaticMeshContainer()
	{
	}
	void StaticMeshContainer::finalize()
	{
		for (auto& mesh : mMeshes)
			mesh->finalize();
	}
	void StaticMeshContainer::init(const std::vector<MeshStore>& stores, const nex::AbstractMaterialLoader & materialLoader)
	{
		for (const auto& store : stores)
		{
			auto mesh = MeshFactory::create(store);
			auto material = materialLoader.createMaterial(store.material);

			add(std::move(mesh), std::move(material));
		}

		setIsLoaded();
	}
	void StaticMeshContainer::add(std::unique_ptr<Mesh> mesh, std::unique_ptr<Material> material)
	{
		auto* pMesh = mesh.get();
		auto* pMaterial = material.get();

		add(std::move(mesh));
		addMaterial(std::move(material));
		addMapping(pMesh, pMaterial);
	}

	void StaticMeshContainer::add(std::unique_ptr<Mesh> mesh)
	{
		assert(mesh != nullptr);
		mMeshes.emplace_back(std::move(mesh));
	}

	void StaticMeshContainer::addMaterial(std::unique_ptr<Material> material)
	{
		assert(material != nullptr);
		mMaterials.emplace_back(std::move(material));
	}

	void StaticMeshContainer::addMapping(Mesh* mesh, Material* material)
	{
		mMappings[mesh] = material;
	}

	SceneNode* StaticMeshContainer::createNodeHierarchyUnsafe(SceneNode* parent)
	{
		std::unique_ptr<SceneNode> root(parent);

		if (!root) {
			root = std::make_unique<SceneNode>();
		}

		for (auto map = mMappings.cbegin(); map != mMappings.cend(); ++map) {
			SceneNode* node = new SceneNode();
			node->setMesh(map->first);
			node->setMaterial(map->second);
			root->addChild(node);
		}

		return root.release();
	}


	const StaticMeshContainer::Mappings& StaticMeshContainer::getMappings() const
	{
		return mMappings;
	}

	const StaticMeshContainer::Materials& StaticMeshContainer::getMaterials() const
	{
		return mMaterials;
	}

	const StaticMeshContainer::Meshes& StaticMeshContainer::getMeshes() const
	{
		return mMeshes;
	}

	void StaticMeshContainer::merge()
	{
		auto materials = collectMaterials();

		for (auto* material : materials) {
			const auto meshes = collectMeshes(material);

			std::vector<Mesh*> meshes32BitIndices;
			std::vector<Mesh*> meshes16BitIndices;

			// we cannot merge meshes with different index element types
			// so we filter them and merge them separately
			std::copy_if(meshes.begin(), meshes.end(), std::back_inserter(meshes32BitIndices),
				[](auto* mesh) {
					return mesh->getIndexBuffer().getType() == IndexElementType::BIT_32;
				});

			std::copy_if(meshes.begin(), meshes.end(), std::back_inserter(meshes16BitIndices),
				[](auto* mesh) {
					return mesh->getIndexBuffer().getType() == IndexElementType::BIT_16;
				});

			auto merged = merge(meshes32BitIndices, material, IndexElementType::BIT_32); 
			if (merged) {
				removeMeshes(meshes32BitIndices);
				addMapping(merged.get(), material);
				add(std::move(merged));
			}

			merged = merge(meshes16BitIndices, material, IndexElementType::BIT_16);
			if (merged) {
				removeMeshes(meshes16BitIndices);

				addMapping(merged.get(), material);
				add(std::move(merged));
			}
		}
		
	}

	std::unique_ptr<nex::Mesh> StaticMeshContainer::merge(const std::vector<Mesh*>& meshes, const Material* material, IndexElementType type)
	{

		if (meshes.size() < 2) return nullptr;

		// compute combined size for vertex and index buffers
		size_t verticesByteSize = 0;
		size_t indicesCount = 0;

		// Note: All meshes have got the same vertex layout since they use the same material (shader)!
		const auto& layout = meshes[0]->getLayout();

		size_t stride = layout.getStride();

		for (auto* mesh : meshes) {
			for (const auto& buffer : mesh->getVertexBuffers()) {
				verticesByteSize += buffer->getSize();
			}
			
			indicesCount += mesh->getIndexBuffer().getCount();

			if (mesh->getIndexBuffer().getType() != type) {
				throw_with_trace(std::runtime_error(
					"StaticMeshContainer::merge : Cannot merge meshes with different index element types!"
				));
			}
		}

		// create merged mesh
		auto vertexBuffer = std::make_unique<VertexBuffer>(verticesByteSize, nullptr);
		IndexBuffer indexBuffer(type, indicesCount, nullptr);


		size_t collectedVerticesBytes = 0;
		size_t collectedIndicesBytes = 0;

		for (auto* mesh : meshes) {

			for (const auto& vBuffer : mesh->getVertexBuffers()) {
				auto* vertexData = vBuffer->map(GpuBuffer::Access::READ_ONLY);
				vertexBuffer->update(vBuffer->getSize(), vertexData, collectedVerticesBytes);
				collectedVerticesBytes += vBuffer->getSize();
				vBuffer->unmap();
			}

			const auto& iBuffer = mesh->getIndexBuffer();

			auto indexOffset = collectedVerticesBytes / stride;
			auto* indexData = iBuffer.map(GpuBuffer::Access::READ_ONLY);
				// We have to translate the indices so that they specify the right vertices
				translate(indexOffset, type, iBuffer.getCount(), indexData);
				indexBuffer.update(iBuffer.getSize(), indexData, collectedIndicesBytes);
				collectedIndicesBytes += iBuffer.getSize();
			iBuffer.unmap();
		}

		//Compute bounding box of meshes 
		AABB box;
		for (auto* mesh : meshes) {
			const auto& mBox = mesh->getAABB();
			box.min = minVec(box.min, mBox.min);
			box.max = maxVec(box.max, mBox.max);
		}


		// All meshes have the same topology TODO
		const auto topology = meshes[0]->getTopology();

		auto merged =  std::make_unique<Mesh>();
		merged->addVertexDataBuffer(std::move(vertexBuffer));
		merged->setBoundingBox(std::move(box));
		merged->setIndexBuffer(std::move(indexBuffer));
		merged->setLayout(std::move(layout));
		merged->setTopology(topology);
		merged->finalize();
		return merged;
	}

	void StaticMeshContainer::removeMeshes(std::vector<Mesh*>& meshes)
	{
		for (auto* mesh : meshes) {
			auto eraseIt = std::remove_if(mMeshes.begin(), mMeshes.end(), [&](auto& mMesh) {
				return mMesh.get() == mesh;
				});

			if (eraseIt != mMeshes.end())
				mMeshes.erase(eraseIt, mMeshes.end());

			// remove all associated mappings
			mMappings.erase(mesh);
		}
	}

	void StaticMeshContainer::translate(size_t offset, IndexElementType type, size_t count, void* data)
	{
		if (type == IndexElementType::BIT_32) {
			unsigned* typedData = (unsigned*)data;
			for (auto i = 0; i < count; ++i) {
				typedData[i] = typedData[i] + offset;
			}
		}
		else {
			unsigned short* typedData = (unsigned short*)data;
			for (auto i = 0; i < count; ++i) {
				typedData[i] = typedData[i] + offset;
			}
		}
	}

	std::vector<Material*> StaticMeshContainer::collectMaterials() const
	{
		std::set<Material*> materials;

		for (const auto& mapping : mMappings) {
			materials.insert(mapping.second);
		}

		return std::vector<Material*>(materials.begin(), materials.end());
	}

	std::vector<Mesh*> StaticMeshContainer::collectMeshes(const Material* material) const
	{
		std::set<Mesh*> meshes;
		for (const auto& mapping : mMappings) {
			if (mapping.second == material)
				meshes.insert(mapping.first);
		}
		return std::vector<Mesh*>(meshes.begin(), meshes.end());
	}

	SceneNode* StaticMesh::createNodeHierarchy(const Mappings& mappings, SceneNode* parent)
	{
		std::unique_ptr<SceneNode> root(parent);

		if (!root) {
			root = std::make_unique<SceneNode>();
		}

		for (auto& mapping : mappings)
		{
			auto* material = mapping.second;
			SceneNode* node = new SceneNode();
			node->setMesh(mapping.first);
			node->setMaterial(material);
			root->addChild(node);
		}

		return root.release();
	}
}