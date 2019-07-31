#include <nex/mesh/StaticMesh.hpp>
#include <nex/Scene.hpp>
#include <nex/material/Material.hpp>
#include "MeshFactory.hpp"
#include "nex/material/AbstractMaterialLoader.hpp"

namespace nex
{
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

		for (auto it = mMeshes.cbegin(); it != mMeshes.cend(); ++it)
		{
			auto* material = mMappings[it->get()];
			SceneNode* node = new SceneNode();
			node->setMesh(it->get());
			node->setMaterial(material);
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