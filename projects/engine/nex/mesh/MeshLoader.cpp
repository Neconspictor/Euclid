#include <nex/mesh/MeshLoader.hpp>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <nex/util/Timer.hpp>
#include <nex/mesh/MeshFactory.hpp>
#include <nex/util/ExceptionHandling.hpp>
#include <nex/material/Material.hpp>

using namespace std;
using namespace glm;

namespace nex
{
	MeshLoader::MeshLoader() : m_logger("MeshLoader")
	{
	}

	unique_ptr<StaticMeshContainer> MeshLoader::loadStaticMesh(const std::filesystem::path& path, const AbstractMaterialLoader& materialLoader) const
	{
		Timer timer;

		Assimp::Importer importer;

		// read the mesh file and triangulate it since processNode expects a triangulated mesh
		const aiScene* scene = importer.ReadFile(path.generic_string(),
			aiProcess_Triangulate
			//| aiProcess_FlipUVs
			| aiProcess_GenSmoothNormals
			| aiProcess_CalcTangentSpace
			| aiProcessPreset_TargetRealtime_MaxQuality);


		if (!scene || scene->mFlags == AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
		{
			LOG(m_logger, nex::Error) << "AssimpModelLoader::loadModel(string): " << importer.GetErrorString();
			stringstream ss;
			ss << "AssimpModelLoader::loadModel(string): Couldn't load mesh: " << path;
			throw_with_trace(runtime_error(ss.str()));
		}

		auto container = make_unique<StaticMeshContainer>();

		processNode(scene->mRootNode, scene, container.get(), materialLoader);

		timer.update();
		LOG(m_logger, nex::Debug) << "Time needed for mesh loading: " << timer.getTimeInSeconds();

		return std::move(container);
	}

	void MeshLoader::processNode(aiNode* node, const aiScene* scene, StaticMeshContainer* container, const AbstractMaterialLoader& materialLoader) const
	{
		// process all the node's meshes (if any)
		for (unsigned i = 0; i < node->mNumMeshes; ++i)
		{
			aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
			processMesh(mesh, scene, container, materialLoader);
		}

		// then do the same for each of its children
		for (unsigned i = 0; i < node->mNumChildren; ++i)
		{
			processNode(node->mChildren[i], scene, container, materialLoader);
		}
	}

	void MeshLoader::processMesh(aiMesh* assimpMesh, const aiScene* scene, StaticMeshContainer* container, const AbstractMaterialLoader& materialLoader) const
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

		bool tangentData = assimpMesh->mTangents != nullptr;

		if (!tangentData) {
			std::runtime_error("No tangent data available!");
		}

		for (unsigned i = 0; i < assimpMesh->mNumVertices; ++i)
		{

			Vertex vertex;
			// position
			vertex.position.x = assimpMesh->mVertices[i].x;
			vertex.position.y = assimpMesh->mVertices[i].y;
			vertex.position.z = assimpMesh->mVertices[i].z;

			// normal
			vertex.normal.x = assimpMesh->mNormals[i].x;
			vertex.normal.y = assimpMesh->mNormals[i].y;
			vertex.normal.z = assimpMesh->mNormals[i].z;

			// tangent
			if (tangentData) {
				vertex.tangent.x = assimpMesh->mTangents[i].x;
				vertex.tangent.y = assimpMesh->mTangents[i].y;
				vertex.tangent.z = assimpMesh->mTangents[i].z;

				vertex.bitangent.x = assimpMesh->mBitangents[i].x;
				vertex.bitangent.y = assimpMesh->mBitangents[i].y;
				vertex.bitangent.z = assimpMesh->mBitangents[i].z;
			}


			// uv
			if (!assimpMesh->mTextureCoords[0]) { // does the mesh contain no uv data?
				vertex.texCoords = { 0.0f,0.0f };
				//texCoords->push_back({0.0f, 0.0f});
			}
			else {
				// A vertex can contain up to 8 different texture coordinates. 
				// We thus make the assumption that we won't 
				// use models (currently!) where a vertex can have multiple texture coordinates 
				// so we always take the first set (0).
				vertex.texCoords.x = assimpMesh->mTextureCoords[0][i].x;
				vertex.texCoords.y = assimpMesh->mTextureCoords[0][i].y;
				//auto x = mesh->mTextureCoords[0][i].x;
				//auto y = mesh->mTextureCoords[0][i].y;
				//texCoords->push_back({ x , y});

			}

			// don't make a copy
			vertices->emplace_back(vertex);
		}

		// now walk through all mesh's faces to retrieve index data. 
		// As the mesh is triangulated, all faces are triangles.
		for (unsigned i = 0; i < assimpMesh->mNumFaces; ++i)
		{
			aiFace face = assimpMesh->mFaces[i];
			for (unsigned j = 0; j < face.mNumIndices; ++j)
			{
				indices->push_back(face.mIndices[j]);
			}
		}

		auto material = materialLoader.loadShadingMaterial(scene, assimpMesh->mMaterialIndex);

		unique_ptr<Mesh> mesh = MeshFactory::create(vertices->data(), assimpMesh->mNumVertices,
			indices->data(), assimpMesh->mNumFaces * 3);

		container->add(std::move(mesh), std::move(material));
	}
}