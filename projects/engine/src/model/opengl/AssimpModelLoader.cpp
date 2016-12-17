#include <model/opengl/AssimpModelLoader.hpp>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <util/Globals.hpp>
#include <platform/logging/GlobalLoggingServer.hpp>
#include <texture/opengl/TextureManagerGL.hpp>
#include <util/Timer.hpp>
#include <mesh/opengl/MeshFactoryGL.hpp>


using namespace std;
using namespace glm;

AssimpModelLoader::AssimpModelLoader() : logClient(platform::getLogServer())
{
	logClient.setPrefix("AssimpModelLoader");
}

ModelGL AssimpModelLoader::loadModel(const string& path) const
{
	Timer timer;
	timer.update();

	string filePath = util::globals::MESHES_PATH +  path;
	Assimp::Importer importer;

	// read the mesh file and triangulate it since processNode expects a triangulated mesh
	const aiScene* scene = importer.ReadFile(filePath, aiProcess_Triangulate);

	if (!scene || scene->mFlags == AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
	{
		LOG(logClient, platform::Error) << "AssimpModelLoader::loadModel(string): " << importer.GetErrorString();
		stringstream ss; 
		ss << "AssimpModelLoader::loadModel(string): Couldn't load mesh: " << path;
		throw runtime_error(ss.str());
	}

	vector<MeshGL> meshes;
	processNode(scene->mRootNode, scene, &meshes);

	LOG(logClient, platform::Debug) << "Time needed for mesh loading: " << timer.update();

	return ModelGL(move(meshes));
}

void AssimpModelLoader::processNode(aiNode* node, const aiScene* scene, vector<MeshGL>* meshes) const
{
	// process all the node's meshes (if any)
	for (GLuint i = 0; i < node->mNumMeshes; ++i)
	{
		aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
		meshes->push_back(processMesh(mesh, scene));
	}

	// then do the same for each of its children
	for (GLuint i = 0; i < node->mNumChildren; ++i)
	{
		processNode(node->mChildren[i], scene, meshes);
	}
}

MeshGL AssimpModelLoader::processMesh(aiMesh* mesh, const aiScene* scene) const
{
	// Vertex and index count can be large -> store temp objects on heap
	auto vertices = make_unique<vector<Vertex>>();
	auto indices = make_unique<vector<unsigned int>>();

	// We set the size of the vectors initially to avoid unnecessary reallocations.
	// It is assumed that the mesh is triangulated, so each face has exactly three indices.
	//vertices->reserve(mesh->mNumVertices);
	//indices->reserve(mesh->mNumFaces * 3);

	Material material;
	TextureManagerGL* manager = TextureManagerGL::get();

	for (GLuint i = 0; i < mesh->mNumVertices; ++i)
	{

		Vertex vertex;
		// position
		vertex.position.x = mesh->mVertices[i].x;
		vertex.position.y = mesh->mVertices[i].y;
		vertex.position.z = mesh->mVertices[i].z;

		// normal
		vertex.normal.x = mesh->mNormals[i].x;
		vertex.normal.y = mesh->mNormals[i].y;
		vertex.normal.z = mesh->mNormals[i].z;

		// uv
		if (!mesh->mTextureCoords[0]) { // does the mesh contain no uv data?
			vertex.texCoords = {0.0f,0.0f};
		} else {
			// A vertex can contain up to 8 different texture coordinates. 
			// We thus make the assumption that we won't 
			// use models (currently!) where a vertex can have multiple texture coordinates 
			// so we always take the first set (0).
			vertex.texCoords.x = mesh->mTextureCoords[0][i].x;
			vertex.texCoords.y = mesh->mTextureCoords[0][i].y;
		}

		// don't make a copy
		vertices->push_back(move(vertex));
	}

	// now walk through all mesh's faces to retrieve index data. 
	// As the mesh is triangulated, all faces are triangles.
	for (GLuint i = 0; i < mesh->mNumFaces; ++i)
	{
		aiFace face = mesh->mFaces[i];
		for (GLuint j = 0; j < face.mNumIndices; ++j)
		{
			indices->push_back(move(face.mIndices[j]));
		}
	}

	// process material (if any available)
	if (mesh->mMaterialIndex >= 0)
	{
		aiMaterial* mat = scene->mMaterials[mesh->mMaterialIndex];

		// a material can have more than one diffuse/specular/normal map,
		// but we only use the first one by now
		vector<string> diffuseMaps = loadMaterialTextures(mat, aiTextureType_DIFFUSE);
		if (diffuseMaps.size())
		{
			material.setDiffuseMap(manager->getImage(diffuseMaps[0]));
		}
		vector<string> emissionMaps = loadMaterialTextures(mat, aiTextureType_EMISSIVE);
		if (emissionMaps.size())
		{
			material.setEmissionMap(manager->getImage(emissionMaps[0]));
		}
		vector<string> specularMaps = loadMaterialTextures(mat, aiTextureType_SPECULAR);
		if (specularMaps.size())
		{
			material.setSpecularMap(manager->getImage(specularMaps[0]));
		}
	}

	material.setShininess(32);

	MeshGL result  = MeshFactoryGL::create(vertices->data(), mesh->mNumVertices, 
											indices->data(), mesh->mNumFaces * 3);
	result.setMaterial(move(material));

	return result;
}

vector<string> AssimpModelLoader::loadMaterialTextures(aiMaterial* mat, aiTextureType type)
{
	vector<string> textures;
	for (GLuint i = 0; i < mat->GetTextureCount(type); ++i)
	{
		aiString texture;
		mat->GetTexture(type, i, &texture);

		// load image into memory
		// Note: assimp textures are local paths!
		TextureManagerGL::get()->getImage(texture.C_Str());
		textures.push_back(texture.C_Str());
	}

	return textures;
}