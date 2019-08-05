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

	class StaticMeshContainer : public nex::Resource
	{
	public:

		using Meshes = std::vector<std::unique_ptr<Mesh>>;
		using Materials = std::vector<std::unique_ptr<Material>>;
		using Mappings = std::unordered_map<Mesh*, Material*>;

		StaticMeshContainer() = default;

		virtual ~StaticMeshContainer();

		void finalize() override;
		void init(const std::vector<MeshStore>& stores, const nex::AbstractMaterialLoader& materialLoader);

		void add(std::unique_ptr<Mesh> mesh, std::unique_ptr<Material> material);
		void add(std::unique_ptr<Mesh> mesh);
		void addMaterial(std::unique_ptr<Material> material);
		void addMapping(Mesh* mesh, Material* material);

		/**
		 * Note: Returned SceneNode* has to be deleted by user!
		 */
		SceneNode* createNodeHierarchyUnsafe(SceneNode* parent = nullptr);

		const Mappings& getMappings() const;
		const Materials& getMaterials() const;
		const Meshes& getMeshes() const;

		/**
		 * Merges meshes with same material.
		 */
		void merge();
		
	protected:

		std::vector<Material*> collectMaterials() const;
		std::vector<Mesh*> collectMeshes(const Material* material) const;

		std::unique_ptr<Mesh> merge(const std::vector<Mesh*>& meshes, const Material* material, IndexElementType type);

		void removeMeshes(std::vector<Mesh*>& meshes);

		void translate(size_t offset, IndexElementType type, size_t count, void* data);

		Mappings mMappings;
		Materials mMaterials;
		Meshes mMeshes;
	};


	class StaticMesh
	{
	public:

		using Mappings = std::unordered_map<Mesh*, Material*>;

		static SceneNode* createNodeHierarchy(const Mappings& mappings, SceneNode* parent = nullptr);
	};
}