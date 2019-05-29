#pragma once
#include <nex/mesh/StaticMesh.hpp>
#include <assimp/scene.h>
#include <nex/material/AbstractMaterialLoader.hpp>
#include <filesystem>
#include <nex/common/Log.hpp>

namespace nex
{


	struct TextureData;
	struct AABB;
	struct MeshStore;

	class MeshLoader
	{
	public:

		using Vertex = Mesh::Vertex;

		MeshLoader();
		std::vector<MeshStore> loadStaticMesh(const std::filesystem::path&  path, const AbstractMaterialLoader& materialLoader) const;

	protected:

		void processNode(aiNode* node, const aiScene* scene, std::vector<MeshStore>& stores, const AbstractMaterialLoader& materialLoader) const;

		static AABB calcBoundingBox(const std::vector<Vertex>& vertices);
		/**
		 * Creates a MeshGL out of an aiMesh. It is assumed that the given aiMesh is triangulated.
		 */
		void processMesh(aiMesh* mesh, const aiScene* scene, std::vector<MeshStore>& stores, const AbstractMaterialLoader& materialLoader) const;

	private:

		nex::Logger m_logger;
	};
}