#pragma once
#include <nex/opengl/model/ModelGL.hpp>
#include <assimp/scene.h>
#include <nex/logging/LoggingClient.hpp>
#include <nex/material/AbstractMaterialLoader.hpp>

struct TextureData;

class AssimpModelLoader
{
public:

	using Vertex = MeshGL::Vertex;

	AssimpModelLoader();
	std::unique_ptr<ModelGL> loadModel(const std::string& path, const AbstractMaterialLoader& materialLoader) const;

protected:

	void processNode(aiNode* node, const aiScene* scene, std::vector<std::unique_ptr<MeshGL>>* resultMeshes, const AbstractMaterialLoader& materialLoader) const;

	/**
	 * Creates a MeshGL out of an aiMesh. It is assumed that the given aiMesh is triangulated.
	 */
	std::unique_ptr<MeshGL> processMesh(aiMesh* mesh, const aiScene* scene, const AbstractMaterialLoader& materialLoader) const;

private:

	nex::LoggingClient logClient;
};