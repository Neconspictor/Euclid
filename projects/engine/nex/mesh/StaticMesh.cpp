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
		assert(mesh != nullptr);
		assert(material != nullptr);

		mMeshes.emplace_back(std::move(mesh));
		mMaterials.emplace_back(std::move(material));

		auto* pMesh = mMeshes.back().get();
		auto* pMaterial = mMaterials.back().get();
		mMappings[pMesh] = pMaterial;
	}

	SceneNode* StaticMeshContainer::createNodeHierarchy(Scene* scene, SceneNode* parent)
	{
		assert(scene != nullptr);

		if (!parent) {
			parent = scene->createNode();
		}

		for (auto it = mMeshes.cbegin(); it != mMeshes.cend(); ++it)
		{
			auto* material = mMappings[it->get()];
			SceneNode* node = scene->createNode();
			node->setMesh(it->get());
			node->setMaterial(material);
			parent->addChild(node);
		}

		return parent;
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

	SceneNode* StaticMesh::createNodeHierarchy(Scene* scene, const Mappings& mappings, SceneNode* parent)
	{
		assert(scene != nullptr);

		if (!parent) {
			parent = scene->createNode();
		}

		for (auto& mapping : mappings)
		{
			auto* material = mapping.second;
			SceneNode* node = scene->createNode();
			node->setMesh(mapping.first);
			node->setMaterial(material);
			parent->addChild(node);
		}

		return parent;
	}
}