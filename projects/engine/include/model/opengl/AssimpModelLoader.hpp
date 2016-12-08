#ifndef ENGINE_MODEL_OPENGL_ASSIMP_MODEL_LOADER_HPP
#define ENGINE_MODEL_OPENGL_ASSIMP_MODEL_LOADER_HPP
#include <model/opengl/ModelGL.hpp>
#include <assimp/scene.h>
#include <platform/logging/LoggingClient.hpp>

class AssimpModelLoader
{
public:
	AssimpModelLoader();
	ModelGL loadModel(const std::string& path) const;

private:
	void processNode(aiNode* node, const aiScene* scene, std::vector<MeshGL>* resultMeshes) const;
	MeshGL processMesh(aiMesh* mesh, const aiScene* scene) const;
	static std::vector<std::string> loadMaterialTextures(aiMaterial* mat, aiTextureType type);

	platform::LoggingClient logClient;
};

#endif