#pragma once
#include <nex/mesh/MeshGroup.hpp>
#include <nex/material/AbstractMaterialLoader.hpp>
#include <filesystem>
#include <nex/common/Log.hpp>
#include <nex/scene/VobStore.hpp>


struct aiScene;

namespace nex
{
	struct TextureDesc;
	struct AABB;
	struct MeshStore;
	class ImportScene;
	class Rig;

	class AbstractMeshLoader
	{
	public:

		using MeshVec = std::vector<std::unique_ptr<MeshStore>>;

		AbstractMeshLoader();
		virtual ~AbstractMeshLoader() = default;
		virtual MeshVec loadMesh(const ImportScene& scene, const AbstractMaterialLoader& materialLoader, float rescale);

		template <typename Vertex>
		static AABB calcBoundingBox(const std::vector<Vertex>& vertices)
		{
			nex::AABB result;
			for (const auto& vertex : vertices)
			{
				result.min = minVec(result.min, vertex.position);
				result.max = maxVec(result.max, vertex.position);
			}

			return result;
		}

		/**
		 * Some mesh types (e.g. SkinnedMesh) need pre-processing with ImportScene data.
		 */
		virtual bool needsPreProcessWithImportScene() const;
		virtual void preProcessInputScene(const ImportScene& scene);

	protected:
		virtual void processNode(const std::filesystem::path&  pathAbsolute, 
			aiNode* node, 
			const aiScene* scene, 
			MeshVec& stores,
			const AbstractMaterialLoader& materialLoader,
			const glm::mat4& parentTrafo) const;

		/**
		 * Creates a Mesh out of an aiMesh. It is assumed that the given aiMesh is triangulated.
		 */
		virtual void processMesh(const std::filesystem::path&  
			pathAbsolute, aiMesh* mesh, 
			const aiScene* scene, 
			MeshVec& stores,
			const AbstractMaterialLoader& materialLoader, const glm::mat4& parentTrafo, const glm::mat3& normalMatrix) const = 0;

		nex::Logger mLogger;
	};

	template<typename T>
	class MeshLoader : public AbstractMeshLoader
	{
	public:
		virtual ~MeshLoader() = default;
	protected:

		using Vertex = T;

		/**
		 * Creates a Mesh out of an aiMesh. It is assumed that the given aiMesh is triangulated.
		 */
		void processMesh(const std::filesystem::path&  pathAbsolute, 
			aiMesh* mesh, 
			const aiScene* scene, 
			MeshVec& stores,
			const AbstractMaterialLoader& materialLoader, const glm::mat4& parentTrafo, const glm::mat3& normalMatrix) const override;
	};

	void nex::MeshLoader<nex::Mesh::Vertex>::processMesh(const std::filesystem::path&  pathAbsolute, 
		aiMesh* mesh, 
		const aiScene* scene, 
		MeshVec& stores,
		const AbstractMaterialLoader& materialLoader, const glm::mat4& parentTrafo, const glm::mat3& normalMatrix) const;

	void nex::MeshLoader<nex::VertexPosition>::processMesh(const std::filesystem::path& pathAbsolute, 
		aiMesh* mesh, 
		const aiScene* scene, 
		MeshVec& stores,
		const AbstractMaterialLoader& materialLoader, const glm::mat4& parentTrafo, const glm::mat3& normalMatrix) const;


	class SkinnedMeshLoader : public AbstractMeshLoader
	{
	public:
		SkinnedMeshLoader() = default;
		virtual ~SkinnedMeshLoader() = default;

		MeshVec loadMesh(const ImportScene& scene, const AbstractMaterialLoader& materialLoader, float rescale) override;

		bool needsPreProcessWithImportScene() const override;
		void preProcessInputScene(const ImportScene& scene) override;

	protected:

		using Vertex = nex::SkinnedVertex;

		/**
		 * Creates a Mesh out of an aiMesh. It is assumed that the given aiMesh is triangulated.
		 */
		void processMesh(const std::filesystem::path& pathAbsolute, 
			aiMesh* mesh, 
			const aiScene* scene, 
			MeshVec& stores,
			const AbstractMaterialLoader& materialLoader,
			const glm::mat4& parentTrafo,
			const glm::mat3& normalMatrix) const override;

		const nex::Rig* mRig = nullptr;
	};


	class MeshProcessor {
	public:
		MeshProcessor(const AbstractMaterialLoader* materialLoader, const std::filesystem::path& meshPath);

		virtual ~MeshProcessor() = default;

		/**
		 * Creates a Mesh out of an aiMesh. It is assumed that the given aiMesh is triangulated.
		 */
		virtual void processMesh(const aiMesh* mesh, VobBaseStore::MeshVec& stores) const = 0;

	protected:
		const AbstractMaterialLoader* mMaterialLoader;
		std::filesystem::path mMeshPathAbsolute;
	};


	class NodeHierarchyLoader {
	public:
		NodeHierarchyLoader(const ImportScene* scene, MeshProcessor* processor);

		template <typename Vertex>
		static AABB calcBoundingBox(const std::vector<Vertex>& vertices)
		{
			nex::AABB result;
			for (const auto& vertex : vertices)
			{
				result.min = minVec(result.min, vertex.position);
				result.max = maxVec(result.max, vertex.position);
			}

			return result;
		}


	protected:
		const ImportScene* mScene; 
		MeshProcessor* mProcessor;

		VobBaseStore::MeshVec collectMeshes(const aiNode* node) const;

		VobBaseStore processNode(const aiNode* node) const;
	};
}