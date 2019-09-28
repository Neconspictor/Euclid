#pragma once
#include <nex/mesh/StaticMesh.hpp>
#include <assimp/scene.h>
#include <nex/material/AbstractMaterialLoader.hpp>
#include <filesystem>
#include <nex/common/Log.hpp>

namespace nex
{


	struct TextureDesc;
	struct AABB;
	struct MeshStore;

	class AbstractMeshLoader
	{
	public:

		AbstractMeshLoader();
		virtual ~AbstractMeshLoader() = default;
		std::vector<MeshStore> loadStaticMesh(const std::filesystem::path&  path, const AbstractMaterialLoader& materialLoader) const;

	protected:
		void processNode(const std::filesystem::path&  pathAbsolute, aiNode* node, const aiScene* scene, std::vector<MeshStore>& stores, const AbstractMaterialLoader& materialLoader) const;

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

		AABB calcBoundingBox(const std::vector<Vertex>& vertices) const;

		/**
		 * Creates a Mesh out of an aiMesh. It is assumed that the given aiMesh is triangulated.
		 */
		void processMesh(const std::filesystem::path&  pathAbsolute, aiMesh* mesh, const aiScene* scene, std::vector<MeshStore>& stores, const AbstractMaterialLoader& materialLoader) const override;
	};

	void nex::MeshLoader<nex::Mesh::Vertex>::processMesh(const std::filesystem::path&  pathAbsolute, aiMesh* mesh, const aiScene* scene, std::vector<MeshStore>& stores,
		const AbstractMaterialLoader& materialLoader) const;
}
