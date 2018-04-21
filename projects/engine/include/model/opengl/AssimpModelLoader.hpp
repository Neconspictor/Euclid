#pragma once
#include <model/opengl/ModelGL.hpp>
#include <assimp/scene.h>
#include <platform/logging/LoggingClient.hpp>

struct TextureData;

class AssimpModelLoader
{
public:

	using Vertex = MeshGL::Vertex;

	AssimpModelLoader();
	std::unique_ptr<ModelGL> loadModel(const std::string& path) const;

private:
	void processNode(aiNode* node, const aiScene* scene, std::vector<std::unique_ptr<MeshGL>>* resultMeshes) const;

	/**
	 * Creates a MeshGL out of an aiMesh. It is assumed that the given aiMesh is triangulated.
	 */
	std::unique_ptr<MeshGL> processMesh(aiMesh* mesh, const aiScene* scene) const;

	static std::vector<std::string> loadMaterialTextures(aiMaterial* mat, aiTextureType type, TextureData data);

	platform::LoggingClient logClient;
};