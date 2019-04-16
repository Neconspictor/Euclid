#pragma once
#include <nex/mesh/StaticMesh.hpp>
#include <assimp/scene.h>
#include <nex/material/AbstractMaterialLoader.hpp>
#include <filesystem>
#include <nex/common/Log.hpp>

namespace nex
{


	struct TextureData;

	class MeshLoader
	{
	public:

		using Vertex = Mesh::Vertex;

		MeshLoader();
		std::unique_ptr<StaticMeshContainer> loadStaticMesh(const std::filesystem::path&  path, const AbstractMaterialLoader& materialLoader) const;

	protected:

		void processNode(aiNode* node, const aiScene* scene, std::vector<std::unique_ptr<Mesh>>* resultMeshes, const std::vector<std::unique_ptr<Material>>& materials) const;

		/**
		 * Creates a MeshGL out of an aiMesh. It is assumed that the given aiMesh is triangulated.
		 */
		std::unique_ptr<Mesh> processMesh(aiMesh* mesh, const aiScene* scene, const std::vector<std::unique_ptr<Material>>& materials) const;

	private:

		nex::Logger m_logger;
	};
}
