#include <nex/opengl/model/AssimpModelLoader.hpp>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <nex/util/Globals.hpp>
#include <nex/logging/GlobalLoggingServer.hpp>
#include <nex/opengl/texture/TextureManagerGL.hpp>
#include <nex/util/Timer.hpp>
#include <nex/opengl/mesh/MeshFactoryGL.hpp>


using namespace std;
using namespace glm;

AssimpModelLoader::AssimpModelLoader() : logClient(nex::getLogServer())
{
	logClient.setPrefix("AssimpModelLoader");
}

unique_ptr<ModelGL> AssimpModelLoader::loadModel(const string& path, const AbstractMaterialLoader& materialLoader) const
{
	Timer timer;
	timer.update();

	string filePath = util::globals::MESHES_PATH +  path;
	Assimp::Importer importer;

	// read the mesh file and triangulate it since processNode expects a triangulated mesh
	const aiScene* scene = importer.ReadFile(filePath, 
		aiProcess_Triangulate 
		//| aiProcess_FlipUVs
		| aiProcess_GenSmoothNormals 
		| aiProcess_CalcTangentSpace
		| aiProcessPreset_TargetRealtime_MaxQuality);


	if (!scene || scene->mFlags == AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
	{
		LOG(logClient, nex::Error) << "AssimpModelLoader::loadModel(string): " << importer.GetErrorString();
		stringstream ss; 
		ss << "AssimpModelLoader::loadModel(string): Couldn't load mesh: " << path;
		throw_with_trace(runtime_error(ss.str()));
	}

	vector<unique_ptr<MeshGL>> meshes;
	processNode(scene->mRootNode, scene, &meshes, materialLoader);

	LOG(logClient, nex::Debug) << "Time needed for mesh loading: " << timer.update();

	return make_unique<ModelGL>(move(meshes));
}

void AssimpModelLoader::processNode(aiNode* node, const aiScene* scene, vector<unique_ptr<MeshGL>>* meshes, const AbstractMaterialLoader& materialLoader) const
{
	// process all the node's meshes (if any)
	for (GLuint i = 0; i < node->mNumMeshes; ++i)
	{
		aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
		meshes->push_back(move(processMesh(mesh, scene, materialLoader)));
	}

	// then do the same for each of its children
	for (GLuint i = 0; i < node->mNumChildren; ++i)
	{
		processNode(node->mChildren[i], scene, meshes, materialLoader);
	}
}

unique_ptr<MeshGL> AssimpModelLoader::processMesh(aiMesh* mesh, const aiScene* scene, const AbstractMaterialLoader& materialLoader) const
{
	// Vertex and index count can be large -> store temp objects on heap
	auto vertices = make_unique<vector<Vertex>>();
	auto indices = make_unique<vector<unsigned int>>();
	//auto texCoords = make_unique<vector<vec2>>();
	//auto positions = make_unique<vector<vec3>>();
	//auto normals = make_unique<vector<vec3>>();
	// We set the size of the vectors initially to avoid unnecessary reallocations.
	// It is assumed that the mesh is triangulated, so each face has exactly three indices.
	//vertices->reserve(mesh->mNumVertices);
	//indices->reserve(mesh->mNumFaces * 3);

	bool tangentData = mesh->mTangents != nullptr;

	if (!tangentData) {
		std::runtime_error("No tangent data available!");
	}

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

		// tangent
		if (tangentData) {
			vertex.tangent.x = mesh->mTangents[i].x;
			vertex.tangent.y = mesh->mTangents[i].y;
			vertex.tangent.z = mesh->mTangents[i].z;

			vertex.bitangent.x = mesh->mBitangents[i].x;
			vertex.bitangent.y = mesh->mBitangents[i].y;
			vertex.bitangent.z = mesh->mBitangents[i].z;
		}


		// uv
		if (!mesh->mTextureCoords[0]) { // does the mesh contain no uv data?
			vertex.texCoords = {0.0f,0.0f};
			//texCoords->push_back({0.0f, 0.0f});
		} else {
			// A vertex can contain up to 8 different texture coordinates. 
			// We thus make the assumption that we won't 
			// use models (currently!) where a vertex can have multiple texture coordinates 
			// so we always take the first set (0).
			vertex.texCoords.x = mesh->mTextureCoords[0][i].x;
			vertex.texCoords.y = mesh->mTextureCoords[0][i].y;
			//auto x = mesh->mTextureCoords[0][i].x;
			//auto y = mesh->mTextureCoords[0][i].y;
			//texCoords->push_back({ x , y});

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


	unique_ptr<Material> material = materialLoader.loadShadingMaterial(mesh, scene);

	unique_ptr<MeshGL> result  = MeshFactoryGL::create(vertices->data(), mesh->mNumVertices, 
											indices->data(), mesh->mNumFaces * 3);

	result->setMaterial(move(material));

	return move(result);
}