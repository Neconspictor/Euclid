#include <nex/anim/RigManager.hpp>
#include <nex/util/ExceptionHandling.hpp>
#include <nex/import/ImportScene.hpp>
#include <nex/resource/FileSystem.hpp>
#include <nex/config/Configuration.hpp>
#include <nex/util/StringUtils.hpp>
#include <nex/exception/ResourceLoadException.hpp>
#include <nex/anim/RigLoader.hpp>
#include <nex/resource/FileSystem.hpp>
#include <nex/util/StringUtils.hpp>

nex::RigManager::~RigManager() = default;

void nex::RigManager::add(std::unique_ptr<Rig> rig)
{
	auto id = rig->getID();
	auto result = mRigs.insert(std::pair<unsigned, std::unique_ptr<Rig>>(id, std::move(rig)));
	if (!result.second) {
		throw_with_trace(std::invalid_argument("nex::RigManager::add : There exists already a rig with id "
			+ std::to_string(id)));
	}
}

const nex::Rig* nex::RigManager::load(const nex::ImportScene& importScene) {

	auto metaFilePath = absolute(importScene.getFilePath());
	metaFilePath += "_meta.ini";

	Configuration meta;
	std::string idOption;
	meta.addOption("Rig", "id", &idOption, std::string());


	if (!meta.load(metaFilePath.generic_string())) {
		throw_with_trace(nex::ResourceLoadException("nex::RigManager::load: meta file couldn't be loaded: " +
			metaFilePath.generic_string()));
	}

	auto strID = meta.get<std::string>("Rig.id");

	if (strID == "") {
		throw_with_trace(nex::ResourceLoadException("nex::RigManager::load: Rig.id not set in meta file or is empty: " +
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
		mFileSystem->store(path, *rig);

	}

	return rig;

}

const nex::Rig* nex::RigManager::getByID(unsigned id) const
{
	auto it = mRigs.find(id);

	if (it != mRigs.end())
		return it->second.get();

	return nullptr;
}

nex::RigManager* nex::RigManager::get()
{
	static RigManager instance;
	return &instance;
}

void nex::RigManager::init(std::string compiledSubFolder, std::string compiledFileExtension)
{
	auto* manager = RigManager::get();
	manager->mFileSystem = std::make_unique<FileSystem>(
		std::vector<std::filesystem::path> {compiledSubFolder},
		compiledSubFolder,
		compiledFileExtension
	);



	auto files = FileSystem::filter(FileSystem::getFilesFromFolder(compiledSubFolder), compiledFileExtension);

	for (auto& file : files) {
		auto rig = std::make_unique<Rig>(Rig::createUninitialized());
		manager->mFileSystem->load(file, *rig);
		manager->add(std::move(rig));
	}

}