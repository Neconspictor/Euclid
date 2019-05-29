#include <nex/mesh/StaticMesh.hpp>
#include <nex/Scene.hpp>
#include <nex/material/Material.hpp>
#include "MeshFactory.hpp"
#include "nex/material/AbstractMaterialLoader.hpp"

namespace nex
{
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

	std::unique_ptr<StaticMeshContainer> StaticMeshContainer::create(const std::vector<MeshStore>& stores, 
		const nex::AbstractMaterialLoader& materialLoader)
	{
		auto container = std::make_unique<StaticMeshContainer>();

		for (const auto& store : stores)
		{
			auto mesh = MeshFactory::create(store);
			auto material = materialLoader.createMaterial(store.material);

			container->add(std::move(mesh), std::move(material));
		}

		return container;
	}

	SceneNode* StaticMeshContainer::createNodeHierarchy(Scene* scene, SceneNode* parent)
	{
		assert(scene != nullptr);

		if (!parent) {
			parent = scene->createNode();
			scene->addRoot(parent);
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
}
