#include <nex/mesh/MeshManager.hpp>
#include <nex/mesh/Mesh.hpp>
#include <nex/mesh/MeshGroup.hpp>
#include <nex/mesh/MeshLoader.hpp>
#include <nex/texture/TextureManager.hpp>
#include <nex/mesh/MeshFactory.hpp>
#include <sstream>
#include <string>
#include <nex/util/StringUtils.hpp>
#include <nex/mesh/SampleMeshes.hpp>
#include <nex/shader/Shader.hpp>
#include <nex/mesh/UtilityMeshes.hpp>
#include "nex/material/Material.hpp"
#include "nex/material/AbstractMaterialLoader.hpp"
#include <nex/resource/FileSystem.hpp>
#include "VertexLayout.hpp"
#include <nex/buffer/VertexBuffer.hpp>
#include "nex/math/Constant.hpp"
#include <nex/resource/ResourceLoader.hpp>
#include <nex/renderer/RenderBackend.hpp>
#include <nex/mesh/UtilityMeshes.hpp>
#include <nex/import/ImportScene.hpp>
#include <nex/anim/AnimationManager.hpp>
#include <nex/anim/RigLoader.hpp>

std::unique_ptr<nex::MeshManager> nex::MeshManager::mInstance;


nex::MeshManager::MeshManager() :
		mFileSystem(nullptr),
		mInitialized(false)
	{
		CUBE_POSITION_NORMAL_TEX_HASH = nex::util::customSimpleHash(sample_meshes::CUBE_POSITION_NORMAL_TEX_NAME);
		SKYBOX_MODEL_HASH = nex::util::customSimpleHash(sample_meshes::SKY_BOX_NAME);
		SPRITE_MODEL_HASH = nex::util::customSimpleHash(sample_meshes::RECTANGLE_NAME);

		mFullscreenPlane = std::make_unique<VertexArray>();
		static const float fullscreenPlaneTriangleStripVerticesOpengl[] = {
			// position 4 floats, texture coords 2 floats
			-1.0, +1.0, 0.0, 1.0, 0.0, 1.0,
			-1.0, -1.0, 0.0, 1.0, 0.0, 0.0,
			+1.0, -1.0, 0.0, 1.0, 1.0, 0.0,
			+1.0, +1.0, 0.0, 1.0, 1.0, 1.0
		};

		mFullscreenTriangleData = std::make_unique<VertexBuffer>(sizeof(fullscreenPlaneTriangleStripVerticesOpengl), fullscreenPlaneTriangleStripVerticesOpengl);
		VertexLayout layout;
		layout.push<float>(4, mFullscreenTriangleData.get(), false, false, true);
		layout.push<float>(2, mFullscreenTriangleData.get(), false, false, true);

		mFullscreenPlane->bind();
		mFullscreenPlane->init(layout);
		mFullscreenPlane->unbind(); // important: In OpenGL implementation VertexBuffer creation with arguments corrupts state of vertex array, if not unbounded!


		mFullscreenTriangle = std::make_unique<VertexArray>();
		static const float fullscreenTriangleVerticesOpengl[] = {
			// position 4 floats, texture coords 2 floats
			-1.0, -1.0, 0.0, 1.0, 0.0, 0.0,
			+3.0, -1.0, 0.0, 1.0, 2.0, 0.0,
			-1.0, +3.0, 0.0, 1.0, 0.0, 2.0
		};

		mFullscreenTriangleData = std::make_unique<VertexBuffer>(sizeof(fullscreenTriangleVerticesOpengl), fullscreenTriangleVerticesOpengl);
		
		layout = VertexLayout();
		layout.push<float>(4, mFullscreenTriangleData.get(), false, false, true);
		layout.push<float>(2, mFullscreenTriangleData.get(), false, false, true);
		
		mFullscreenTriangle->bind();
		mFullscreenTriangle->init(layout);
		mFullscreenTriangle->unbind();


		AABB unitBox = {glm::vec3(-1.0f), glm::vec3(1.0f)};
		mUnitBoundingBoxLines = std::make_unique<MeshAABB>(unitBox, Topology::LINES);
		mUnitBoundingBoxLines->finalize();
		mUnitBoundingBoxTriangles = std::make_unique<MeshAABB>(unitBox, Topology::TRIANGLES);
		mUnitBoundingBoxTriangles->finalize();
		mUnitSphereTriangles = std::make_unique<SphereMesh>(16, 16, true);
	}

nex::MeshManager::~MeshManager() 
{

}

std::unique_ptr<nex::MeshGroup> nex::MeshManager::createSphere(unsigned xSegments, unsigned ySegments,
	std::unique_ptr<Material> material)
{
	auto mesh = std::make_unique<SphereMesh>(xSegments, ySegments);
	auto model = std::make_unique<MeshGroup>();
	model->add(std::move(mesh), std::move(material));
	model->finalize();

	return model;
}

nex::MeshGroup* nex::MeshManager::getSkyBox()
	{
		using Vertex = VertexPosition;

		using namespace nex::util;

		auto it = modelTable.find(SKYBOX_MODEL_HASH);
		if (it == modelTable.end())
		{
			int vertexCount = (int)sizeof(sample_meshes::skyBoxVertices);
			int indexCount = (int)sizeof(sample_meshes::skyBoxIndices);

			AABB boundingBox;
			boundingBox.min = glm::vec3(0.0f);
			boundingBox.max = glm::vec3(0.0f);

			std::unique_ptr<Mesh> mesh = MeshFactory::createPosition((const Vertex*)sample_meshes::skyBoxVertices, vertexCount,
				sample_meshes::skyBoxIndices, (int)indexCount, std::move(boundingBox));

			auto model = std::make_unique<MeshGroup>();
			model->add(std::move(mesh), std::make_unique<Material>(nullptr));

			models.push_back(move(model));
			MeshGroup* result = models.back().get();
			result->finalize();

			modelTable[SKYBOX_MODEL_HASH] = result;

			result->finalize();

			return result;
		}

		return it->second;
	}

	nex::MeshGroup* nex::MeshManager::getSprite()
	{
		using Vertex = VertexPositionTex;

		auto it = modelTable.find(SPRITE_MODEL_HASH);
		if (it != modelTable.end())
		{
			return dynamic_cast<MeshGroup*>(it->second);
		}

		// create a Quad mesh that fills up the enter screen; normalized device coordinates range from [-1, 1] in x,y and z axis;
		// as we want a  2D model, the z axis is ignored/set to 1.0f
		// normal vectors aren't needed, too -> set to 0.0f as well.
		std::vector<Vertex> vertices;
		Vertex vertex;

		// left upper corner 
		vertex.position = { 0.0f,  0.0f, 0.0f };
		vertex.texCoords = { 0.0f, 1.0f };
		vertices.push_back(vertex);

		// left bottom corner
		vertex.position = { 0.0f,  1.0, 0.0f };
		vertex.texCoords = { 0.0f, 0.0f };
		vertices.push_back(vertex);

		// right bottom corner
		vertex.position = { 1.0f,  1.0f, 0.0f };
		vertex.texCoords = { 1.0f, 0.0f };
		vertices.push_back(vertex);

		// right upper corner
		vertex.position = { 1.0f,  0.0f, 0.0f };
		vertex.texCoords = { 1.0f, 1.0f };
		vertices.push_back(vertex);

		// create index buffer in counter clock winding order
		std::vector<unsigned int> indices;

		// first triangle
		indices.push_back(0);
		indices.push_back(1);
		indices.push_back(3);

		// second triangle
		indices.push_back(1);
		indices.push_back(2);
		indices.push_back(3);

		AABB boundingBox;
		boundingBox.min = glm::vec3(0.0f);
		boundingBox.max = glm::vec3(0.0f);

		std::unique_ptr<Mesh> mesh = MeshFactory::createPositionUV(vertices.data(), (int)vertices.size(),
			indices.data(), (int)indices.size(), std::move(boundingBox));

		auto model = std::make_unique<MeshGroup>();
		model->add(std::move(mesh), nullptr);

		models.push_back(std::move(model));

		MeshGroup* result = models.back().get();
		modelTable[SPRITE_MODEL_HASH] = result;

		result->finalize();

		return result;
	}

	nex::MeshAABB* nex::MeshManager::getUnitBoundingBoxLines()
	{
		return mUnitBoundingBoxLines.get();
	}

	nex::MeshAABB* nex::MeshManager::getUnitBoundingBoxTriangles()
	{
		return mUnitBoundingBoxTriangles.get();
	}

	nex::SphereMesh* nex::MeshManager::getUnitSphereTriangles()
	{
		return mUnitSphereTriangles.get();
	}

	void nex::MeshManager::init(std::filesystem::path meshRootPath,
		std::string compiledRootFolder,
		std::string compiledFileExtension)
	{

		mInstance = std::make_unique<MeshManager>();

		std::vector<std::filesystem::path> includeDirectories = { std::move(meshRootPath) };
		mInstance->mFileSystem = std::make_unique<FileSystem>(std::move(includeDirectories), std::move(compiledRootFolder), std::move(compiledFileExtension));
		mInstance->mInitialized = true;
	}

	nex::MeshGroup* nex::MeshManager::loadModel(const std::filesystem::path& meshPath,
		const nex::AbstractMaterialLoader& materialLoader,
		AbstractMeshLoader* meshLoader,
		const FileSystem* fileSystem)
	{
		// else case: assume the model name is a 3d model that can be load from file.
		if (!mInitialized) throw std::runtime_error("MeshManager isn't initialized!");

		if (!fileSystem) fileSystem = mFileSystem.get();

		const auto resolvedPath = fileSystem->resolvePath(meshPath);
		auto hash = nex::util::customSimpleHash(resolvedPath.u8string());

		auto it = modelTable.find(hash);
		if (it != modelTable.end())
		{
			return it->second;
		}

		if (hash == SPRITE_MODEL_HASH)
		{
			return getSprite();
		}

		if (hash == SKYBOX_MODEL_HASH)
		{
			return getSkyBox();
		}

		MeshLoader<Mesh::Vertex> defaultMeshLoader;
		if (meshLoader == nullptr) {
			meshLoader = &defaultMeshLoader;
		}

		const std::function<void(AbstractMeshLoader::MeshVec&)> loader = [&](auto& meshes)->void
		{
			auto importScene = ImportScene::read(resolvedPath);
			meshes = meshLoader->loadMesh(importScene, materialLoader);
		};

		AbstractMeshLoader::MeshVec stores;
		bool forceLoad = false;
		auto compiledPath = fileSystem->getCompiledPath(resolvedPath).path;

		if (!std::filesystem::exists(compiledPath) || forceLoad)
		{
			//const auto resolvedPath = resolvePath(resourcePath);
			loader(stores);
			auto* ptr = stores[0].get();


			BinStream file;
			auto directory = compiledPath.parent_path();
			std::filesystem::create_directories(directory);

			std::ios_base::openmode mode = std::ios::out | std::ios::trunc;

			file.open(compiledPath, mode);

			file << stores.size();
			for (auto& store : stores) {
				store->write(file);
			}
		}
		else
		{
			BinStream file;
			file.open(compiledPath, std::ios::in);

			size_t size;
			file >> size;
			stores = meshLoader->createMeshStoreVector(size);

			for (int i = 0; i < size; ++i) {
				stores[i]->read(file);
			}
		}

		auto mesh = std::make_unique<MeshGroup>();
		MeshGroup* result = mesh.get();
		result->init(stores, materialLoader);


		models.emplace_back(std::move(mesh));
		modelTable[hash] = result;

		return result;
	}

nex::VertexArray* nex::MeshManager::getNDCFullscreenPlane()
{
	return mFullscreenPlane.get();
}

nex::VertexArray* nex::MeshManager::getNDCFullscreenTriangle()
{
	return mFullscreenTriangle.get();
}


nex::MeshGroup* nex::MeshManager::getPositionNormalTexCube()
{
	using Vertex = VertexPositionNormalTex;

	auto it = modelTable.find(CUBE_POSITION_NORMAL_TEX_HASH);
	if (it != modelTable.end())
	{
		return it->second;
	}

	std::vector<Vertex> vertices;
	unsigned int vertexCount = sizeof(sample_meshes::cubePositionNormalTexVertices) / sizeof(Vertex);
	unsigned int vertexSlice = sizeof(Vertex) / sizeof(float);
	for (unsigned int i = 0; i < vertexCount; ++i)
	{
		Vertex vertex;
		const float* source = &sample_meshes::cubePositionNormalTexVertices[i * vertexSlice];
		vertex.position = { *(source), *(source + 1), *(source + 2) };
		source += 3;
		vertex.normal = { *(source), *(source + 1), *(source + 2) };
		source += 3;
		vertex.texCoords = { *(source), *(source + 1) };
		vertices.emplace_back(std::move(vertex));
	}

	std::vector<unsigned int> indices;
	unsigned int indexCount = sizeof(sample_meshes::cubePositionNormalTexIndices) / sizeof(unsigned int);
	for (unsigned int i = 0; i < indexCount; ++i)
	{
		indices.push_back(sample_meshes::cubePositionNormalTexIndices[i]);
	}

	AABB boundingBox;
	boundingBox.min = glm::vec3(0.0f);
	boundingBox.max = glm::vec3(0.0f);

	std::unique_ptr<Mesh> mesh = MeshFactory::create(vertices.data(), (int)vertices.size(),
		indices.data(), (int)indices.size(), std::move(boundingBox));


	auto model = std::make_unique<MeshGroup>();
	model->add(std::move(mesh), nullptr);

	models.push_back(std::move(model));

	MeshGroup* result = models.back().get();
	modelTable[CUBE_POSITION_NORMAL_TEX_HASH] = result;

	result->finalize();

	return result;
}

nex::MeshManager* nex::MeshManager::get()
{
	return mInstance.get();
}

void nex::MeshManager::loadModels()
{
	//TODO
	getPositionNormalTexCube();


	//AssimpModelLoader::loadModel("");
}

void nex::MeshManager::release()
{
	mInstance.reset(nullptr);
}