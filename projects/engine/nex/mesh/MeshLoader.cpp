#include <nex/mesh/MeshLoader.hpp>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <nex/util/Timer.hpp>
#include <nex/util/ExceptionHandling.hpp>
#include <nex/material/Material.hpp>
#include <nex/mesh/MeshStore.hpp>
#include <nex/math/Math.hpp>
#include <nex/import/ImportScene.hpp>
#include <nex\anim\RigLoader.hpp>
#include <nex/anim/AnimationManager.hpp>

using namespace std;
using namespace glm;

nex::AbstractMeshLoader::AbstractMeshLoader() : mLogger("MeshLoader")
{
}

nex::AbstractMeshLoader::MeshVec nex::AbstractMeshLoader::loadMesh(const ImportScene& scene,
	const AbstractMaterialLoader& materialLoader, float rescale)
{
	MeshVec stores;
	const auto* aiscene = scene.getAssimpScene();
	auto meshFileAbsolute = std::filesystem::canonical(scene.getFilePath());
	auto parentTrafo = glm::mat4(1.0f);
	parentTrafo[0][0] = rescale;
	parentTrafo[1][1] = rescale;
	parentTrafo[2][2] = rescale;
	processNode(meshFileAbsolute, aiscene->mRootNode, aiscene, stores, materialLoader, parentTrafo);

	return stores;
}

nex::AbstractMeshLoader::MeshVec nex::AbstractMeshLoader::createMeshStoreVector(size_t size) const
{
	MeshVec vec(size);
	for (int i = 0; i < size; ++i) {
		vec[i] = std::make_unique<MeshStore>();
	}
	return vec;
}

bool nex::AbstractMeshLoader::needsPreProcessWithImportScene() const
{
	return false;
}

void nex::AbstractMeshLoader::preProcessInputScene(const ImportScene& scene)
{
}

void nex::AbstractMeshLoader::processNode(const std::filesystem::path&  pathAbsolute,
	aiNode* node, 
	const aiScene* scene, 
	MeshVec& stores,
	const AbstractMaterialLoader& materialLoader,
	const glm::mat4& parentTrafo) const
{

	auto trafo = parentTrafo * nex::ImportScene::convert(node->mTransformation);
	auto normalMatrix = nex::createNormalMatrix(trafo);

	// process all the node's meshes (if any)
	for (unsigned i = 0; i < node->mNumMeshes; ++i)
	{
		aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
		processMesh(pathAbsolute, mesh, scene, stores, materialLoader, trafo, normalMatrix);
	}

	// then do the same for each of its children
	for (unsigned i = 0; i < node->mNumChildren; ++i)
	{
		processNode(pathAbsolute, node->mChildren[i], scene, stores, materialLoader, trafo);
	}
}

template <typename Vertex>
void nex::MeshLoader<Vertex>::processMesh(const std::filesystem::path&  pathAbsolute, 
	aiMesh* mesh, 
	const aiScene* scene, 
	MeshVec& stores,
	const AbstractMaterialLoader& materialLoader, 
	const glm::mat4& parentTrafo,
	const glm::mat3& normalMatrix) const
{
	//Note: Explicit instantiation has to be implemented!
	static_assert(false);
}

void nex::MeshLoader<nex::Mesh::Vertex>::processMesh(const std::filesystem::path&  pathAbsolute, 
	aiMesh* mesh, 
	const aiScene* scene, 
	MeshVec& stores,
	const AbstractMaterialLoader& materialLoader, 
	const glm::mat4& parentTrafo,
	const glm::mat3& normalMatrix) const
{
	stores.emplace_back(std::make_unique<MeshStore>());
	auto& store = *stores.back();

	store.indexType = IndexElementType::BIT_32;
	auto& layout = store.layout;

	// Note: we later set the vertex buffer, so nullptr is ok for now
	layout.push<glm::vec3>(1, nullptr, false, false, true); // position
	layout.push<glm::vec3>(1, nullptr, false, false, true); // normal
	layout.push<glm::vec2>(1, nullptr, false, false, true); // uv
	layout.push<glm::vec3>(1, nullptr, false, false, true); // tangent

	store.topology = Topology::TRIANGLES;

	store.arrayOffset = 0;
	store.vertexCount = mesh->mNumVertices;
	store.useIndexBuffer = true;

	store.verticesMap.clear();
	std::vector<Vertex>& vertices = reinterpret_cast<std::vector<Vertex>&>(store.verticesMap[nullptr]);
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

		vertex.position = parentTrafo * glm::vec4(vertex.position, 1.0f);

		// normal
		vertex.normal.x = mesh->mNormals[i].x;
		vertex.normal.y = mesh->mNormals[i].y;
		vertex.normal.z = mesh->mNormals[i].z;
		vertex.normal = normalize(normalMatrix * vertex.normal);

		// tangent
		if (tangentData) {
			vertex.tangent.x = mesh->mTangents[i].x;
			vertex.tangent.y = mesh->mTangents[i].y;
			vertex.tangent.z = mesh->mTangents[i].z;
			vertex.tangent = normalize(normalMatrix * vertex.tangent);
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


void nex::MeshLoader<nex::VertexPosition>::processMesh(const std::filesystem::path&  pathAbsolute, 
	aiMesh* mesh, 
	const aiScene* scene, 
	MeshVec& stores,
	const AbstractMaterialLoader& materialLoader, 
	const glm::mat4& parentTrafo,
	const glm::mat3& normalMatrix) const
{
	
	stores.emplace_back(std::make_unique<MeshStore>());
	auto& store = *stores.back();

	store.indexType = IndexElementType::BIT_32;
	auto& layout = store.layout;

	// Note: we later set the vertex buffer, so nullptr is ok for now
	layout.push<glm::vec3>(1, nullptr, false, false, true); // position

	store.topology = Topology::TRIANGLES;

	store.arrayOffset = 0;
	store.vertexCount = mesh->mNumVertices;
	store.useIndexBuffer = true;

	store.verticesMap.clear();
	std::vector<Vertex>& vertices = reinterpret_cast<std::vector<Vertex>&>(store.verticesMap[nullptr]);
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
		vertex.position = parentTrafo * glm::vec4(vertex.position, 1.0f);

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

nex::AbstractMeshLoader::MeshVec nex::SkinnedMeshLoader::loadMesh(const ImportScene& scene, const AbstractMaterialLoader& materialLoader, float rescale)
{
	if (!mRig) nex::throw_with_trace(std::logic_error("SkinnedMeshLoader needs pre-processing! Fix that bug!"));
	return AbstractMeshLoader::loadMesh(scene, materialLoader, rescale);
}

nex::AbstractMeshLoader::MeshVec nex::SkinnedMeshLoader::createMeshStoreVector(size_t size) const
{
	MeshVec vec(size);
	for (int i = 0; i < size; ++i) {
		vec[i] = std::make_unique<SkinnedMeshStore>();
	}
	return vec;
}

bool nex::SkinnedMeshLoader::needsPreProcessWithImportScene() const
{
	return true;
}

void nex::SkinnedMeshLoader::preProcessInputScene(const ImportScene& scene)
{
	mRig = AnimationManager::get()->load(scene);
}

void nex::SkinnedMeshLoader::processMesh(const std::filesystem::path& pathAbsolute, 
	aiMesh* mesh, 
	const aiScene* scene, 
	MeshVec& stores,
	const AbstractMaterialLoader& materialLoader, 
	const glm::mat4& parentTrafo, 
	const glm::mat3& normalMatrix) const
{
	stores.emplace_back(std::make_unique<SkinnedMeshStore>());
	SkinnedMeshStore& store = (SkinnedMeshStore&)*stores.back();

	store.indexType = IndexElementType::BIT_32;
	auto& layout = store.layout;

	// Note: we later set the vertex buffer, so nullptr is ok for now
	layout.push<glm::vec3>(1, nullptr, false, false, true); // position
	layout.push<glm::vec3>(1, nullptr, false, false, true); // normal
	layout.push<glm::vec2>(1, nullptr, false, false, true); // uv
	layout.push<glm::vec3>(1, nullptr, false, false, true); // tangent
	layout.push<glm::uvec4>(1, nullptr, false, false, false); // boneIDs
	layout.push<glm::vec4>(1, nullptr, false, false, true); // boneWeights

	store.topology = Topology::TRIANGLES;
	store.arrayOffset = 0;
	store.vertexCount = mesh->mNumVertices;
	store.useIndexBuffer = true;

	store.verticesMap.clear();
	std::vector<Vertex>& vertices = reinterpret_cast<std::vector<Vertex>&>(store.verticesMap[nullptr]);
	std::vector<unsigned>& indices = reinterpret_cast<std::vector<unsigned>&>(store.indices);

	vertices.resize(mesh->mNumVertices);
	indices.resize(mesh->mNumFaces * 3);

	std::vector<unsigned> counters (mesh->mNumVertices, 0);

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
		// we don't have to transform skinned meshes
		//vertex.position = parentTrafo * glm::vec4(vertex.position, 1.0f);

		// normal
		vertex.normal.x = mesh->mNormals[i].x;
		vertex.normal.y = mesh->mNormals[i].y;
		vertex.normal.z = mesh->mNormals[i].z;
		//vertex.normal = normalize(normalMatrix * vertex.normal);

		// tangent
		if (tangentData) {
			vertex.tangent.x = mesh->mTangents[i].x;
			vertex.tangent.y = mesh->mTangents[i].y;
			vertex.tangent.z = mesh->mTangents[i].z;
			//vertex.tangent = normalize(normalMatrix * vertex.tangent);
		}


		// uv
		if (!mesh->mTextureCoords[0]) { // does the mesh contain no uv data?
			vertex.texCoords = { 0.0f,0.0f };
		}
		else {
			// A vertex can contain up to 8 different texture coordinates. 
			// We thus make the assumption that we won't 
			// use models (currently!) where a vertex can have multiple texture coordinates 
			// so we always take the first set (0).
			vertex.texCoords.x = mesh->mTextureCoords[0][i].x;
			vertex.texCoords.y = mesh->mTextureCoords[0][i].y;
		}

		//TODO bone id and vertex weights
		// Note bone Id and weights are applied later

		vertices[i] = std::move(vertex);
	}

	// Retrieve bone ids and weights
	for (int i = 0; i < mesh->mNumBones; ++i) {
		auto* bone = mesh->mBones[i];

		for (int j = 0; j < bone->mNumWeights; ++j) {
			auto& weight = bone->mWeights[j];
			auto id = weight.mVertexId;
			auto& vertex = vertices[id];

			//retrieve index from temporarily counter
			auto index = counters[id];

			if (index >= 4) {
				throw_with_trace(nex::ResourceLoadException(
					"nex::SkinnedMeshLoader::processMesh : Vertices with more than 4 assigned bones aren't supported!"));
			}

			++counters[id];

			std::string boneName = bone->mName.C_Str();
			auto* bone2 = mRig->getByName(boneName);
			if (bone2 == nullptr) {
				throw_with_trace(nex::ResourceLoadException(
					"nex::SkinnedMeshLoader::processMesh : No bone exists with name:  " + boneName));
			}
			vertex.boneIDs[index] = bone2->getID();
			vertex.boneWeights[index] = weight.mWeight;
		}
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
	store.rigID = mRig->getID();
}