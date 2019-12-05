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



	// unit plane
	static const float unitPlaneVertices[] = {
		// position 2 floats
		-0.5f, 0.5f,
		-0.5f, -0.5f,
		0.5f, 0.5f,
		0.5f, -0.5
	};

	auto unitPlaneVB = std::make_unique<VertexBuffer>(sizeof(unitPlaneVertices), unitPlaneVertices);

	layout = VertexLayout();
	layout.push<float>(2, unitPlaneVB.get(), false, false, true);

	mUnitPlane = std::make_unique<Mesh>();
	mUnitPlane->addVertexDataBuffer(std::move(unitPlaneVB));
	mUnitPlane->setLayout(std::move(layout));
	mUnitPlane->setTopology(Topology::TRIANGLE_STRIP);
	mUnitPlane->setUseIndexBuffer(false);
	mUnitPlane->setVertexCount(sizeof(unitPlaneVertices) / (2 * sizeof(float)));
	mUnitPlane->finalize();
}

nex::MeshManager::~MeshManager() = default;

std::unique_ptr<nex::MeshGroup> nex::MeshManager::createSphere(unsigned xSegments, unsigned ySegments,
	std::unique_ptr<Material> material)
{
	auto mesh = std::make_unique<SphereMesh>(xSegments, ySegments);
	auto model = std::make_unique<MeshGroup>();
	model->add(std::move(mesh), std::move(material));
	model->calcBatches();
	model->finalize();

	return model;
}

nex::MeshAABB* nex::MeshManager::getUnitBoundingBoxLines()
{
	return mUnitBoundingBoxLines.get();
}

nex::MeshAABB* nex::MeshManager::getUnitBoundingBoxTriangles()
{
	return mUnitBoundingBoxTriangles.get();
}

const nex::Mesh* nex::MeshManager::getUnitPlane() const
{
	return mUnitPlane.get();
}

nex::SphereMesh* nex::MeshManager::getUnitSphereTriangles()
{
	return mUnitSphereTriangles.get();
}

void nex::MeshManager::init(const std::filesystem::path& meshRootPath,
	const std::string& compiledRootFolder,
	const std::string& compiledFileExtension)
{

	mInstance = std::make_unique<MeshManager>();

	std::vector<std::filesystem::path> includeDirectories = { std::move(meshRootPath) };
	mInstance->mFileSystem = std::make_unique<FileSystem>(includeDirectories, compiledRootFolder, compiledFileExtension);
	mInstance->mInitialized = true;
}

std::unique_ptr<nex::MeshGroup> nex::MeshManager::loadModel(const std::filesystem::path& meshPath,
	const nex::AbstractMaterialLoader& materialLoader,
	AbstractMeshLoader* meshLoader,
	const FileSystem* fileSystem)
{
	// else case: assume the model name is a 3d model that can be load from file.
	if (!mInitialized) throw std::runtime_error("MeshManager isn't initialized!");

	if (!fileSystem) fileSystem = mFileSystem.get();

	const auto resolvedPath = fileSystem->resolvePath(meshPath);

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

	auto group = std::make_unique<MeshGroup>();
	group->init(stores, materialLoader);

	return group;
}

nex::VertexArray* nex::MeshManager::getNDCFullscreenPlane()
{
	return mFullscreenPlane.get();
}

nex::VertexArray* nex::MeshManager::getNDCFullscreenTriangle()
{
	return mFullscreenTriangle.get();
}

nex::MeshManager* nex::MeshManager::get()
{
	return mInstance.get();
}

void nex::MeshManager::release()
{
	mInstance.reset(nullptr);
}