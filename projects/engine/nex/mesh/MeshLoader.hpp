#pragma once
#include <nex/mesh/StaticMesh.hpp>
#include <nex/material/AbstractMaterialLoader.hpp>
#include <filesystem>
#include <nex/common/Log.hpp>


struct aiScene;

namespace nex
{


	struct TextureDesc;
	struct AABB;
	struct MeshStore;
	class ImportScene;

	class AbstractMeshLoader
	{
	public:

		AbstractMeshLoader();
		virtual ~AbstractMeshLoader() = default;
		virtual std::vector<MeshStore> loadMesh(const ImportScene& scene, const AbstractMaterialLoader& materialLoader) const;

	protected:
		virtual void processNode(const std::filesystem::path&  pathAbsolute, 
			aiNode* node, 
			const aiScene* scene, 
			std::vector<MeshStore>& stores, 
			const AbstractMaterialLoader& materialLoader) const;

		/**
		 * Creates a Mesh out of an aiMesh. It is assumed that the given aiMesh is triangulated.
		 */
		virtual void processMesh(const std::filesystem::path&  pathAbsolute, aiMesh* mesh, const aiScene* scene, std::vector<MeshStore>& stores, const AbstractMaterialLoader& materialLoader) const = 0;

		nex::Logger mLogger;
	};

	template<typename T>
	class MeshLoader : public AbstractMeshLoader
	{
	public:
		virtual ~MeshLoader() = default;
	protected:

		using Vertex = T;

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
		 * Creates a Mesh out of an aiMesh. It is assumed that the given aiMesh is triangulated.
		 */
		void processMesh(const std::filesystem::path&  pathAbsolute, aiMesh* mesh, const aiScene* scene, std::vector<MeshStore>& stores, const AbstractMaterialLoader& materialLoader) const override;
	};

	void nex::MeshLoader<nex::Mesh::Vertex>::processMesh(const std::filesystem::path&  pathAbsolute, aiMesh* mesh, const aiScene* scene, std::vector<MeshStore>& stores,
		const AbstractMaterialLoader& materialLoader) const;

	void nex::MeshLoader<nex::VertexPosition>::processMesh(const std::filesystem::path& pathAbsolute, aiMesh* mesh, const aiScene* scene, std::vector<MeshStore>& stores,
		const AbstractMaterialLoader& materialLoader) const;


	class SkinnedMeshLoader : public AbstractMeshLoader
	{
	public:
		virtual ~SkinnedMeshLoader() = default;
	protected:

		using Vertex = nex::SkinnedVertex;

		void processNode(const std::filesystem::path& pathAbsolute,
			aiNode* node,
			const aiScene* scene,
			std::vector<MeshStore>& stores,
			const AbstractMaterialLoader& materialLoader) const override;

		/**
		 * Creates a Mesh out of an aiMesh. It is assumed that the given aiMesh is triangulated.
		 */
		void processMesh(const std::filesystem::path& pathAbsolute, 
			aiMesh* mesh, 
			const aiScene* scene, 
			std::vector<MeshStore>& stores, 
			const AbstractMaterialLoader& materialLoader) const override;
	};
}
