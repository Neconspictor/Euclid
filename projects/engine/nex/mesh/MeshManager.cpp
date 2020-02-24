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
	static const float fullscreenPlaneQuadStripVerticesOpengl[] = {
		// position 4 floats, texture coords 2 floats
		-1.0, +1.0, 0.0, 1.0, 0.0, 1.0,
		-1.0, -1.0, 0.0, 1.0, 0.0, 0.0,
		+1.0, -1.0, 0.0, 1.0, 1.0, 0.0,
		+1.0, +1.0, 0.0, 1.0, 1.0, 1.0
	};

	mFullscreenPlaneData = std::make_unique<VertexBuffer>(sizeof(fullscreenPlaneQuadStripVerticesOpengl), fullscreenPlaneQuadStripVerticesOpengl);
	VertexLayout layout;
	layout.push<float>(4, mFullscreenPlaneData.get(), false, false, true);
	layout.push<float>(2, mFullscreenPlaneData.get(), false, false, true);

	mFullscreenPlane->setLayout(layout);
	mFullscreenPlane->init();
	mFullscreenPlane->unbind(); // important: In OpenGL implementation VertexBuffer creation with arguments corrupts state of vertex array, if not unbounded!


	mFullscreenTriangle = std::make_unique<VertexArray>();
	static const float fullscreenTriangleVerticesOpengl[] = {
		// position 4 floats, texture coords 2 floats
		-1.0f, -1.0f, 0.0f, 1.0f, 0.0f, 0.0f,
		+3.0f, -1.0f, 0.0f, 1.0f, 2.0f, 0.0f,
		-1.0f, +3.0f, 0.0f, 1.0f, 0.0f, 2.0f
	};

	mFullscreenTriangleData = std::make_unique<VertexBuffer>(sizeof(fullscreenTriangleVerticesOpengl), fullscreenTriangleVerticesOpengl);
		
	layout = VertexLayout();
	layout.push<float>(4, mFullscreenTriangleData.get(), false, false, true);
	layout.push<float>(2, mFullscreenTriangleData.get(), false, false, true);
		
	mFullscreenTriangle->setLayout(layout);
	mFullscreenTriangle->init();
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
	mUnitPlane->getVertexArray().setLayout(layout);
	mUnitPlane->addVertexDataBuffer(std::move(unitPlaneVB));
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

void nex::MeshManager::init(const std::filesystem::path& resourceRootFolder,
	const std::string& compiledRootFolder,
	const std::string& compiledFileExtension)
{

	mInstance = std::make_unique<MeshManager>();

	std::vector<std::filesystem::path> includeDirectories = { std::move(resourceRootFolder) };
	mInstance->mFileSystem = std::make_unique<FileSystem>(includeDirectories, compiledRootFolder, compiledFileExtension);
	mInstance->mInitialized = true;
}

std::unique_ptr<nex::Vob> nex::MeshManager::loadVobHierarchy(const std::filesystem::path& meshPath,
	const nex::AbstractMaterialLoader& materialLoader, 
	float rescale, 
	bool forceLoad, 
	const FileSystem* fileSystem)
{

	// else case: assume the model name is a 3d model that can be load from file.
	if (!mInitialized) throw std::runtime_error("MeshManager isn't initialized!");

	if (!fileSystem) fileSystem = mFileSystem.get();

	const auto resolvedPath = fileSystem->resolvePath(meshPath);

	VobBaseStore store;
	auto compiledPath = constructCompiledPath(resolvedPath, fileSystem, rescale, ".CVOB");
	const bool compiledPathExists = std::filesystem::exists(compiledPath);

	ImportScene importScene;

	// Load import scene if needed
	if (!compiledPathExists || forceLoad) {
		importScene = ImportScene::read(resolvedPath, true);
	}

	// Load stores
	if (!compiledPathExists || forceLoad) {

		NodeHierarchyLoader loader(&importScene, &materialLoader);
		store = loader.load(AnimationManager::get());
	}
	else
	{
		BinStream file;
		file.open(compiledPath, std::ios::in);
		file >> store;
	}

	// Write compilation if it doesn't exist already
	if (!compiledPathExists) {
		BinStream file;
		auto directory = compiledPath.parent_path();
		std::filesystem::create_directories(directory);

		std::ios_base::openmode mode = std::ios::out | std::ios::trunc;

		file.open(compiledPath, mode);

		file << store;
	}

	auto vob = createVob(store, materialLoader);
	vob->updateTrafo(true, true);

	nex::RenderEngine::getCommandQueue()->push([vob =vob.get()]() {
		vob->finalizeMeshes();
	});

	return vob;
}

const nex::FileSystem& nex::MeshManager::getFileSystem() const
{
	return *mFileSystem;
}

nex::VertexArray* nex::MeshManager::getNDCFullscreenPlane()
{
	return mFullscreenPlane.get();
}

nex::VertexArray* nex::MeshManager::getNDCFullscreenTriangle()
{
	return mFullscreenTriangle.get();
}

nex::VertexBuffer* nex::MeshManager::getNDCFullscreenTriangleVB()
{
	return mFullscreenTriangleData.get();
}

nex::MeshManager* nex::MeshManager::get()
{
	return mInstance.get();
}

void nex::MeshManager::release()
{
	mInstance.reset(nullptr);
}

std::filesystem::path nex::MeshManager::constructCompiledPath(const std::filesystem::path& absolutePath, 
	const FileSystem* filesystem, 
	float rescale, 
	const char* extension
	)
{
	auto compiledPath = filesystem->getCompiledPath(absolutePath, extension).path;;

	if (rescale != 1.0f) {
		auto extension = compiledPath.extension();
		auto filename = compiledPath.filename();
		filename = filename.replace_extension("");
		filename.replace_filename(filename.c_str() + std::wstring(L".scale=") + std::to_wstring(rescale));
		filename.replace_extension(filename.extension().generic_wstring() + extension.generic_wstring());

		compiledPath.replace_filename(filename);
	}

	return compiledPath;
}

std::unique_ptr<nex::Vob> nex::MeshManager::createVob(const VobBaseStore& store, const AbstractMaterialLoader& materialLoader) const
{
	std::unique_ptr<Vob> vob;

	std::string rigID;
	bool isSkinned = checkIsSkinned(store, rigID);
	if (isSkinned) {
		vob = std::make_unique<RiggedVob>();
		auto* rig = AnimationManager::get()->load(rigID);
	}
	else {
	 vob = std::make_unique<Vob>();
	}

	//collect mesh group
	auto group = std::make_unique<nex::MeshGroup>();
	group->init(store.meshes, materialLoader);

	vob->setMeshGroup(std::move(group));

	vob->setTrafoLocalToParent(store.localToParentTrafo);

	for (auto& childStore : store.children) {
		auto childVob = createVob(childStore, materialLoader);
		childVob->setParent(vob.get());
		vob->addChild(std::move(childVob));
	}


	return vob;
}

bool nex::MeshManager::checkIsSkinned(const VobBaseStore& store, std::string& rigIDOut) const
{
	for (const auto& mesh : store.meshes) {
		if (mesh.isSkinned) {
			rigIDOut = mesh.rigID;
			return true;
		}
	}

	return false;
}