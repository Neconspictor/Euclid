#include <nex/anim/AnimationManager.hpp>
#include <nex/util/ExceptionHandling.hpp>
#include <nex/import/ImportScene.hpp>
#include <nex/resource/FileSystem.hpp>
#include <nex/config/Configuration.hpp>
#include <nex/util/StringUtils.hpp>
#include <nex/exception/ResourceLoadException.hpp>
#include <nex/anim/RigLoader.hpp>
#include <nex/resource/FileSystem.hpp>
#include <nex/util/StringUtils.hpp>
#include <nex/mesh/StaticMesh.hpp>

nex::AnimationManager::~AnimationManager() = default;

void nex::AnimationManager::add(std::unique_ptr<Rig> rig)
{
	auto id = rig->getID();
	auto result = mRigs.insert(std::pair<unsigned, std::unique_ptr<Rig>>(id, std::move(rig)));
	if (!result.second) {
		throw_with_trace(std::invalid_argument("nex::AnimationManager::add : There exists already a rig with id "
			+ std::to_string(id)));
	}
}

const nex::Rig* nex::AnimationManager::load(const nex::ImportScene& importScene) {

	auto metaFilePath = absolute(importScene.getFilePath());
	metaFilePath += "_meta.ini";

	Configuration meta;
	std::string idOption;
	meta.addOption("Rig", "id", &idOption, std::string());


	if (!meta.load(metaFilePath.generic_string())) {
		throw_with_trace(nex::ResourceLoadException("nex::AnimationManager::load: meta file couldn't be loaded: " +
			metaFilePath.generic_string()));
	}

	auto strID = meta.get<std::string>("Rig.id");

	if (strID == "") {
		throw_with_trace(nex::ResourceLoadException("nex::AnimationManager::load: Rig.id not set in meta file or is empty: " +
			metaFilePath.generic_string()));
	}


	auto id = SID(strID);

	auto* rig = getByID(id);
	if (rig == nullptr) {
		RigLoader loader;
		auto loadedRig = loader.load(importScene, id);
		add(std::move(loadedRig));
		rig = getByID(id);
		assert(rig != nullptr);


		auto path = mFileSystem->getCompiledPath(strID).path;
		FileSystem::store(path, *rig);

	}

	return rig;
}

const nex::Rig* nex::AnimationManager::getByID(unsigned id) const
{
	auto it = mRigs.find(id);

	if (it != mRigs.end())
		return it->second.get();

	return nullptr;
}

const nex::Rig* nex::AnimationManager::getRig(const MeshContainer& container) const {
	
	for (const auto& mesh : container.getMeshes()) {
		auto* skinnedVersion = dynamic_cast<const SkinnedMesh*>(mesh.get());
		if (skinnedVersion) {
			auto id = skinnedVersion->getRigID();
			return getByID(id);
		}
	}

	throw_with_trace(std::invalid_argument("container has no associated rig!"));
}

const nex::FileSystem* nex::AnimationManager::getFileSystem() const
{
	return mFileSystem.get();
}

nex::AnimationManager* nex::AnimationManager::get()
{
	static AnimationManager instance;
	return &instance;
}

void nex::AnimationManager::init(
	const std::filesystem::path& animationRootPath,
	const std::string& compiledSubFolder, 
	const std::string& compiledAnimationFileExtension,
	const std::string& compiledRigFileExtension)
{
	auto* manager = AnimationManager::get();
	manager->mFileSystem = std::make_unique<FileSystem>(
		std::vector<std::filesystem::path> {animationRootPath},
		compiledSubFolder,
		compiledAnimationFileExtension
	);

	auto files = FileSystem::filter(FileSystem::getFilesFromFolder(compiledSubFolder), compiledRigFileExtension);

	for (auto& file : files) {
		auto rig = std::make_unique<Rig>(Rig::createUninitialized());
		FileSystem::load(file, *rig);
		manager->add(std::move(rig));
	}
}