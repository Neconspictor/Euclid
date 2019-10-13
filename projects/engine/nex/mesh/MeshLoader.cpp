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
	AbstractMeshLoader::AbstractMeshLoader() : mLogger("MeshLoader")
	{
	}

	std::vector<MeshStore> AbstractMeshLoader::loadStaticMesh(const std::filesystem::path& path,
		const AbstractMaterialLoader& materialLoader) const
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
			LOG(mLogger, nex::Error) << "loadStaticMesh: " << importer.GetErrorString();
			stringstream ss;
			ss << "loadStaticMesh: Couldn't load mesh: " << path;
			throw_with_trace(runtime_error(ss.str()));
		}

		std::vector<MeshStore> stores;

		auto meshDirectoryAbsolute = std::filesystem::canonical(path.parent_path());

		processNode(meshDirectoryAbsolute, scene->mRootNode, scene, stores, materialLoader);

		timer.update();
		LOG(mLogger, nex::Debug) << "Time needed for mesh loading: " << timer.getTimeInSeconds();

		return stores;
	}

	void AbstractMeshLoader::processNode(const std::filesystem::path&  pathAbsolute, aiNode* node, const aiScene* scene, std::vector<MeshStore>& stores,
		const AbstractMaterialLoader& materialLoader) const
	{
		// process all the node's meshes (if any)
		for (unsigned i = 0; i < node->mNumMeshes; ++i)
		{
			aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
			processMesh(pathAbsolute, mesh, scene, stores, materialLoader);
		}

		// then do the same for each of its children
		for (unsigned i = 0; i < node->mNumChildren; ++i)
		{
			processNode(pathAbsolute, node->mChildren[i], scene, stores, materialLoader);
		}
	}
}

template <typename Vertex>
nex::AABB nex::MeshLoader<Vertex>::calcBoundingBox(const std::vector<Vertex>& vertices) const
{
	nex::AABB result;
	for (const auto& vertex : vertices)
	{
		result.min = minVec(result.min, vertex.position);
		result.max = maxVec(result.max, vertex.position);
	}

	return result;
}

template <typename Vertex>
void nex::MeshLoader<Vertex>::processMesh(const std::filesystem::path&  pathAbsolute, aiMesh* mesh, const aiScene* scene, std::vector<MeshStore>& stores,
	const AbstractMaterialLoader& materialLoader) const
{
	//Note: Explicit instantiation has to be implemented!
	static_assert(false);
}

void nex::MeshLoader<nex::Mesh::Vertex>::processMesh(const std::filesystem::path&  pathAbsolute, aiMesh* mesh, const aiScene* scene, std::vector<MeshStore>& stores,
	const AbstractMaterialLoader& materialLoader) const
{
	stores.emplace_back(MeshStore());
	auto& store = stores.back();

	store.indexType = IndexElementType::BIT_32;
	auto& layout = store.layout;

	// Note: we later set the vertex buffer, so nullptr is ok for now
	layout.push<glm::vec3>(1, nullptr); // position
	layout.push<glm::vec3>(1, nullptr); // normal
	layout.push<glm::vec2>(1, nullptr); // uv
	layout.push<glm::vec3>(1, nullptr); // tangent

	store.topology = Topology::TRIANGLES;

	std::vector<Vertex>& vertices = reinterpret_cast<std::vector<Vertex>&>(store.vertices);
	std::vector<unsigned>& indices = reinterpret_cast<std::vector<unsigned>&>(store.indices);

	vertices.resize(mesh->mNumVertices);
	indices.resize(mesh->mNumFaces * 3);

	bool tangentData = mesh->mTangents != nullptr;

	if (!tangentData) {
		std::runtime_error("No tangent data available!");
	}

	for (unsigned i = 0; i < mesh->mNumVertices; ++i)
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
		}


		// uv
		if (!mesh->mTextureCoords[0]) { // does the mesh contain no uv data?
			vertex.texCoords = { 0.0f,0.0f };
			//texCoords->push_back({0.0f, 0.0f});
		}
		else {
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
		vertices[i] = std::move(vertex);
	}

	// now walk through all mesh's faces to retrieve index data. 
	// As the mesh is triangulated, all faces are triangles.
	for (unsigned i = 0; i < mesh->mNumFaces; ++i)
	{
		aiFace face = mesh->mFaces[i];
		assert(face.mNumIndices == 3);

		for (unsigned j = 0; j < face.mNumIndices; ++j)
		{
			indices[i * 3 + j] = face.mIndices[j];
		}
	}

	materialLoader.loadShadingMaterial(pathAbsolute, scene, store.material, mesh->mMaterialIndex);
	store.boundingBox = calcBoundingBox(vertices);
}


void nex::MeshLoader<nex::VertexPosition>::processMesh(const std::filesystem::path&  pathAbsolute, aiMesh* mesh, const aiScene* scene, std::vector<MeshStore>& stores,
	const AbstractMaterialLoader& materialLoader) const
{
	
	stores.emplace_back(MeshStore());
	auto& store = stores.back();

	store.indexType = IndexElementType::BIT_32;
	auto& layout = store.layout;

	// Note: we later set the vertex buffer, so nullptr is ok for now
	layout.push<glm::vec3>(1, nullptr); // position

	store.topology = Topology::TRIANGLES;

	std::vector<Vertex>& vertices = reinterpret_cast<std::vector<Vertex>&>(store.vertices);
	std::vector<unsigned>& indices = reinterpret_cast<std::vector<unsigned>&>(store.indices);

	vertices.resize(mesh->mNumVertices);
	indices.resize(mesh->mNumFaces * 3);

	for (unsigned i = 0; i < mesh->mNumVertices; ++i)
	{
		Vertex vertex;
		// position
		vertex.position.x = mesh->mVertices[i].x;
		vertex.position.y = mesh->mVertices[i].y;
		vertex.position.z = mesh->mVertices[i].z;

		// don't make a copy
		vertices[i] = std::move(vertex);
	}

	// now walk through all mesh's faces to retrieve index data. 
	// As the mesh is triangulated, all faces are triangles.
	for (unsigned i = 0; i < mesh->mNumFaces; ++i)
	{
		aiFace face = mesh->mFaces[i];
		assert(face.mNumIndices == 3);

		for (unsigned j = 0; j < face.mNumIndices; ++j)
		{
			indices[i * 3 + j] = face.mIndices[j];
		}
	}

	materialLoader.loadShadingMaterial(pathAbsolute, scene, store.material, mesh->mMaterialIndex);
	store.boundingBox = calcBoundingBox(vertices);
}