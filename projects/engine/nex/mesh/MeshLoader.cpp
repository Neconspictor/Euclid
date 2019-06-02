#include <nex/mesh/MeshLoader.hpp>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <nex/util/Timer.hpp>
#include <nex/util/ExceptionHandling.hpp>
#include <nex/material/Material.hpp>
#include <nex/mesh/MeshStore.hpp>
#include <nex/math/Math.hpp>

using namespace std;
using namespace glm;

namespace nex
{
	MeshLoader::MeshLoader() : m_logger("MeshLoader")
	{
	}

	std::vector<MeshStore> MeshLoader::loadStaticMesh(const std::filesystem::path& path, const AbstractMaterialLoader& materialLoader) const
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

		std::vector<MeshStore> stores;
		processNode(scene->mRootNode, scene, stores, materialLoader);

		timer.update();
		LOG(m_logger, nex::Debug) << "Time needed for mesh loading: " << timer.getTimeInSeconds();

		return stores;
	}

	void MeshLoader::processNode(aiNode* node, const aiScene* scene, std::vector<MeshStore>& stores, const AbstractMaterialLoader& materialLoader) const
	{
		// process all the node's meshes (if any)
		for (unsigned i = 0; i < node->mNumMeshes; ++i)
		{
			aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
			processMesh(mesh, scene, stores, materialLoader);
		}

		// then do the same for each of its children
		for (unsigned i = 0; i < node->mNumChildren; ++i)
		{
			processNode(node->mChildren[i], scene, stores, materialLoader);
		}
	}

	AABB MeshLoader::calcBoundingBox(const std::vector<Vertex>& vertices)
	{
		AABB result;
		for (const auto& vertex : vertices)
		{
			result.min = minVec(result.min, vertex.position);
			result.max = maxVec(result.max, vertex.position);
		}

		return result;
	}

	void MeshLoader::processMesh(aiMesh* assimpMesh, const aiScene* scene, std::vector<MeshStore>& stores, const AbstractMaterialLoader& materialLoader) const
	{
		stores.emplace_back(MeshStore());
		auto& store = stores.back();

		store.indexType = IndexElementType::BIT_32;
		auto& layout = store.layout;
		layout.push<glm::vec3>(1); // position
		layout.push<glm::vec3>(1); // normal
		layout.push<glm::vec2>(1); // uv
		layout.push<glm::vec3>(1); // tangent
		layout.push<glm::vec3>(1); // bitangent

		store.topology = Topology::TRIANGLES;

		std::vector<Vertex>& vertices = reinterpret_cast<std::vector<Vertex>&>(store.vertices);
		std::vector<unsigned>& indices = reinterpret_cast<std::vector<unsigned>&>(store.indices);

		vertices.resize(assimpMesh->mNumVertices);
		indices.resize(assimpMesh->mNumFaces * 3);

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
			vertices[i] = std::move(vertex);
		}

		// now walk through all mesh's faces to retrieve index data. 
		// As the mesh is triangulated, all faces are triangles.
		for (unsigned i = 0; i < assimpMesh->mNumFaces; ++i)
		{
			aiFace face = assimpMesh->mFaces[i];
			assert(face.mNumIndices == 3);

			for (unsigned j = 0; j < face.mNumIndices; ++j)
			{
				indices[i*3 + j] = face.mIndices[j];
			}
		}

		materialLoader.loadShadingMaterial(scene, store.material, assimpMesh->mMaterialIndex);
		store.boundingBox = calcBoundingBox(vertices);
		
		//unique_ptr<Mesh> mesh = MeshFactory::create(*store);
		//mesh->setBoundingBox(store.boundingBox);
	}
}