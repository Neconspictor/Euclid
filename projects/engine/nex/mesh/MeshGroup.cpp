#include <nex/mesh/MeshGroup.hpp>
#include <nex/scene/Scene.hpp>
#include <nex/material/Material.hpp>
#include "MeshFactory.hpp"
#include "nex/material/AbstractMaterialLoader.hpp"
#include <nex/math/Math.hpp>

namespace nex
{
	MeshGroup::~MeshGroup() = default;

	void MeshGroup::finalize()
	{
		for (auto& mesh : mMeshes)
			mesh->finalize();
	}
	void MeshGroup::init(const std::vector<std::unique_ptr<MeshStore>>& stores, const nex::AbstractMaterialLoader & materialLoader)
	{
		for (const auto& store : stores)
		{
			auto mesh = MeshFactory::create(store.get());
			auto material = materialLoader.createMaterial(store->material);

			add(std::move(mesh), std::move(material));
		}

		calcBatches();

		setIsLoaded();
	}
	void MeshGroup::add(std::unique_ptr<Mesh> mesh, std::unique_ptr<Material> material)
	{
		auto* pMesh = mesh.get();
		auto* pMaterial = material.get();

		add(std::move(mesh));
		addMaterial(std::move(material));
		addMapping(pMesh, pMaterial);
	}

	void MeshGroup::add(std::unique_ptr<Mesh> mesh)
	{
		assert(mesh != nullptr);
		mMeshes.emplace_back(std::move(mesh));
	}

	void MeshGroup::addMaterial(std::unique_ptr<Material> material)
	{
		assert(material != nullptr);
		mMaterials.emplace_back(std::move(material));
	}

	void MeshGroup::addMapping(Mesh* mesh, Material* material)
	{
		mMappings[mesh] = material;
	}

	std::list<MeshBatch> MeshGroup::createBatches() const
	{
		std::list<MeshBatch> batches;
		MeshBatch::MaterialComparator materialCmp;

		// sort meshes by materials' shaders and render states
		std::set<MeshBatch::Entry, MeshBatch::EntryComparator> entries;
		auto mappingSize = mMappings.size();
		for (const auto& mapping : mMappings) {
			entries.insert(MeshBatch::Entry(mapping.first, mapping.second));
		}

		// Early exit if no entries 
		if (entries.size() == 0) return batches;

		// add the first batch 
		auto* entryMaterial = entries.begin()->second;
		batches.push_back(MeshBatch(entryMaterial->getShader(), entryMaterial->getRenderState()));
		auto it = batches.begin();


		const Material* currentMaterial = entryMaterial;

		// iterate over all entries and batch them if materials' shaders and render states are equal
		for (const auto& entry : entries) {
			auto* material = entry.second;

			bool areEqual = materialCmp(entry.second, currentMaterial) && materialCmp(currentMaterial, entry.second);

			if (areEqual) {
				// add mesh to current active batch
				it->add(entry.first, material);
			} else { 
				// create new batch
				batches.push_back(MeshBatch(material->getShader(), material->getRenderState()));
				it = --batches.end();
			}
		}

		// calculate bounding boxes for each batch
		for (auto& batch : batches) {
			batch.calcBoundingBox();
		}

		return batches;
	}

	const MeshGroup::Mappings& MeshGroup::getMappings() const
	{
		return mMappings;
	}

	const MeshGroup::Materials& MeshGroup::getMaterials() const
	{
		return mMaterials;
	}

	const MeshGroup::Meshes& MeshGroup::getMeshes() const
	{
		return mMeshes;
	}

	std::list<MeshBatch>* MeshGroup::getBatches()
	{
		return &mBatches;
	}

	const std::list<MeshBatch>* MeshGroup::getBatches() const
	{
		return &mBatches;
	}

	void MeshGroup::calcBatches()
	{
		mBatches = createBatches();
	}

	void MeshGroup::merge()
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

	std::unique_ptr<nex::Mesh> MeshGroup::merge(const std::vector<Mesh*>& meshes, const Material* material, IndexElementType type)
	{

		if (meshes.size() < 2) return nullptr;

		bool useIndexBuffer = meshes[0]->getUseIndexBuffer();
		// assert that all meshes to be merged have the same useIndexBuffer setting
		for (const auto* mesh : meshes) {
			if (mesh->getUseIndexBuffer() != useIndexBuffer) {
				throw_with_trace(std::invalid_argument("Cannot merge meshes with different 'useIndexBuffer' setting!"));
			}
		}


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
					"MeshGroup::merge : Cannot merge meshes with different index element types!"
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
		merged->setVertexCount(verticesByteSize / stride);
		merged->setUseIndexBuffer(useIndexBuffer);
		merged->finalize();
		return merged;
	}

	void MeshGroup::removeMeshes(std::vector<Mesh*>& meshes)
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

	void MeshGroup::translate(size_t offset, IndexElementType type, size_t count, void* data)
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

	std::vector<Material*> MeshGroup::collectMaterials() const
	{
		std::set<Material*> materials;

		for (const auto& mapping : mMappings) {
			materials.insert(mapping.second);
		}

		return std::vector<Material*>(materials.begin(), materials.end());
	}

	std::vector<Mesh*> MeshGroup::collectMeshes(const Material* material) const
	{
		std::set<Mesh*> meshes;
		for (const auto& mapping : mMappings) {
			if (mapping.second == material)
				meshes.insert(mapping.first);
		}
		return std::vector<Mesh*>(meshes.begin(), meshes.end());
	}

	MeshBatch::MeshBatch(Shader* shader, RenderState state) : mMaterial(shader)
	{
		mMaterial.getRenderState() = state;
	}

	MeshBatch::~MeshBatch() = default;

	const AABB& MeshBatch::getBoundingBox() const
	{
		return mBoundingBox;
	}

	const std::vector<MeshBatch::Entry>& MeshBatch::getMeshes() const
	{
		return mMeshes;
	}

	Shader* MeshBatch::getShader() const
	{
		return mMaterial.getShader();
	}

	const RenderState& MeshBatch::getState() const
	{
		return mMaterial.getRenderState();
	}
	
	void MeshBatch::add(const Mesh* mesh, const Material* material)
	{
		if (mesh == nullptr) throw_with_trace(std::invalid_argument("MeshBatch::add : mesh mustn't be null!"));
		if (material == nullptr) throw_with_trace(std::invalid_argument("MeshBatch::add : material mustn't be null!"));
		
		if (!(MaterialComparator()(&mMaterial, material) && MaterialComparator()(material, &mMaterial))) {
			throw_with_trace(std::invalid_argument("MeshBatch::add : material doesn't match mesh batch material"));
		}

		mMeshes.push_back({mesh, material});
	}
	
	void MeshBatch::calcBoundingBox()
	{
		mBoundingBox = AABB();
		for (const auto& pair : mMeshes) {
			mBoundingBox = maxAABB(mBoundingBox, pair.first->getAABB());
		}
	}

	bool MeshBatch::EntryComparator::operator()(const Entry& a, const Entry& b) const
	{
		// first sort by material
		bool smaller = MaterialComparator()(a.second, b.second);
		bool equal = smaller && MaterialComparator()(b.second, a.second);
		if (!equal) return smaller;

		// ... finally by mesh; This is necessary since we don't want drop different meshes!
		return a.first < b.first;
	}
	bool MeshBatch::MaterialComparator::operator()(const Material* a, const Material* b) const
	{
		if (a == nullptr && b == nullptr) return true;
		if (a == nullptr || b == nullptr) return false;

		const auto* shader1 = a->getShader();
		const auto* shader2 = b->getShader();

		// sort first by shader
		if (shader1 != shader2) {
			return shader1 < shader2;
		}

		// ... than by render state
		return RenderState::Comparator()(a->getRenderState(), b->getRenderState());
	}
}