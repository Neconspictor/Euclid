#pragma once
#include <model/opengl/ModelGL.hpp>
#include <assimp/scene.h>
#include <platform/logging/LoggingClient.hpp>

class AssimpModelLoader
{
public:

	using Vertex = MeshGL::Vertex;

	AssimpModelLoader();
	ModelGL loadModel(const std::string& path) const;

private:
	void processNode(aiNode* node, const aiScene* scene, std::vector<MeshGL>* resultMeshes) const;

	/**
	 * Creates a MeshGL out of an aiMesh. It is assumed that the given aiMesh is triangulated.
	 */
	MeshGL processMesh(aiMesh* mesh, const aiScene* scene) const;
	static std::vector<std::string> loadMaterialTextures(aiMaterial* mat, aiTextureType type);

	platform::LoggingClient logClient;
};