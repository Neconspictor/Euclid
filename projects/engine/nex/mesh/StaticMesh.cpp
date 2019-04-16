#include <nex/mesh/StaticMesh.hpp>
#include <nex/Scene.hpp>
#include <nex/material/Material.hpp>

namespace nex
{
	void StaticMeshContainer::add(std::unique_ptr<Mesh> mesh, std::unique_ptr<Material> material)
	{
		mMeshes.emplace_back(std::move(mesh));
		mMaterials.emplace_back(std::move(material));

		auto* pMesh = mMeshes.back().get();
		auto* pMaterial = mMaterials.back().get();
		mMappings[pMesh] = pMaterial;
	}

	void StaticMeshContainer::addToNode(SceneNode* parent, Scene* scene)
	{
		assert(parent != nullptr);
		assert(scene != nullptr);

		for (auto it = mMeshes.cbegin(); it != mMeshes.cend(); ++it)
		{
			auto* material = mMappings[it->get()];
			SceneNode* node = scene->createNode();
			node->setMesh(it->get());
			node->setMaterial(material);
			parent->addChild(node);
		}
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
