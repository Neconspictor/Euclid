#pragma once
#include <nex/mesh/Mesh.hpp>
#include <unordered_map>
#include <memory>
#include <nex/mesh/MeshStore.hpp>


namespace nex
{
	class AbstractMaterialLoader;
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

		static std::unique_ptr<StaticMeshContainer> create(const std::vector<MeshStore>& stores, const nex::AbstractMaterialLoader& materialLoader);

		SceneNode* createNodeHierarchy(Scene* scene, SceneNode* parent = nullptr);

		const Mappings& getMappings() const;
		const Materials& getMaterials() const;
		const Meshes& getMeshes() const;
		
	protected:
		Mappings mMappings;
		Materials mMaterials;
		Meshes mMeshes;
	};


	class StaticMesh
	{
	public:

		using Mappings = std::unordered_map<Mesh*, Material*>;

		static SceneNode* createNodeHierarchy(Scene* scene, const Mappings& mappings, SceneNode* parent = nullptr);
	};
}