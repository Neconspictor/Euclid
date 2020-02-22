#pragma once
#include <nex/mesh/Mesh.hpp>
#include <nex/material/Material.hpp>
#include <unordered_map>
#include <memory>
#include <nex/mesh/MeshStore.hpp>


namespace nex
{
	class AbstractMaterialLoader;
	class SceneNode;
	class Scene;
	class Material;
	class Shader;

	/**
	 * A batch of meshes having the same render state and shader
	 */
	class MeshBatch {
	public:

		using Entry = std::pair<Mesh*, Material*>;

		/**
		 * A comparator class for mesh/material entries.
		 * Can be used for sorting mesh and materials and thus creating batches.
		 */
		struct EntryComparator {
			bool operator()(const Entry& a, const Entry& b) const;
		};

		/**
		 * Comparator for materials. Materials are said to be equal if they use the same shader
		 * and have the same render state. Other data is not considered.
		 */
		struct MaterialComparator {
			bool operator()(const Material* a, const Material* b) const;
		};

		MeshBatch(Material* referenceMaterial = nullptr);
		MeshBatch(const MeshBatch&) = default;
		MeshBatch(MeshBatch&&) = default;
		~MeshBatch();

		void add(Mesh* mesh, Material* material);
		void calcBoundingBox();

		const AABB& getBoundingBox() const;
		const std::vector<Entry>& getEntries() const;

		Material* getReferenceMaterial();
		const Material* getReferenceMaterial() const;
		Shader* getShader() const;
		const RenderState& getState() const;

		void setReferenceMaterial(Material* referenceMaterial);

	private:
		std::vector<Entry> mMeshes;
		Material* mMaterial;
		AABB mBoundingBox;
	};

	class MeshGroup : public nex::Resource
	{
	public:

		using Meshes = std::vector<std::unique_ptr<Mesh>>;
		using Materials = std::vector<std::unique_ptr<Material>>;
		using Mappings = std::unordered_map<Mesh*, Material*>;

		MeshGroup() = default;

		virtual ~MeshGroup();

		void finalize() override;
		void init(const std::vector<MeshStore>& stores, const nex::AbstractMaterialLoader& materialLoader);

		void add(std::unique_ptr<Mesh> mesh, std::unique_ptr<Material> material);
		void add(std::unique_ptr<Mesh> mesh);
		void addMaterial(std::unique_ptr<Material> material);
		void addMapping(Mesh* mesh, Material* material);

		const Mappings& getMappings() const;
		const Materials& getMaterials() const;
		const Meshes& getEntries() const;

		std::vector<MeshBatch>* getBatches();
		const std::vector<MeshBatch>* getBatches() const;

		void calcBatches();

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

		/**
		 * Batches meshes having equal shader and render state
		 */
		std::vector<MeshBatch> createBatches() const;

		Mappings mMappings;
		Materials mMaterials;
		Meshes mMeshes;
		std::vector<MeshBatch> mBatches;
	};
}