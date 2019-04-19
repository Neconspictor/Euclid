#pragma once
#include <nex/mesh/SubMesh.hpp>
#include <unordered_map>
#include <memory>


namespace nex
{
	class SceneNode;
	class Scene;
	class Material;

	class StaticMeshContainer
	{
	public:

		using Meshes = std::vector<std::unique_ptr<Mesh>>;
		using Materials = std::vector<std::unique_ptr<Material>>;
		using Mappings = std::unordered_map<Mesh*, Material*>;

		StaticMeshContainer() = default;

		StaticMeshContainer(const StaticMeshContainer&) = delete;
		StaticMeshContainer& operator=(const StaticMeshContainer& o) = delete;

		StaticMeshContainer(StaticMeshContainer&&) = default;
		StaticMeshContainer& operator=(StaticMeshContainer&&) = default;

		~StaticMeshContainer() = default;

		void add(std::unique_ptr<Mesh> mesh, std::unique_ptr<Material> material);

		SceneNode* createNodeHierarchy(Scene* scene, SceneNode* parent = nullptr);

		const Mappings& getMappings() const;
		const Materials& getMaterials() const;
		const Meshes& getMeshes() const;
		
	protected:
		Mappings mMappings;
		Materials mMaterials;
		Meshes mMeshes;
	};
}