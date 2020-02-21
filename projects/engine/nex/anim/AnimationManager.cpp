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
#include <nex/mesh/MeshGroup.hpp>
#include <nex/anim/BoneAnimationLoader.hpp>

nex::AnimationManager::~AnimationManager() = default;

void nex::AnimationManager::add(std::unique_ptr<Rig> rig)
{
	auto* rigPtr = rig.get();
	const auto& strID = rig->getID();
	auto result = mRigs.insert(std::pair<unsigned, std::unique_ptr<Rig>>(SID(strID), std::move(rig)));
	if (!result.second) {
		throw_with_trace(std::invalid_argument("nex::AnimationManager::add : There exists already a rig with id "
			+ strID));
	}

	//create a new entry for linking bone animations to the rig
	mRigToBoneAnimations.insert({ rigPtr, {} });
}

const nex::Rig* nex::AnimationManager::load(const nex::ImportScene& importScene) {

	const auto info = loadRigInfo(importScene);
	const auto id = SID(info.id);

	auto* rig = getBySID(id);
	if (rig == nullptr) {
		RigLoader loader;
		auto loadedRig = loader.load(importScene, info);
		add(std::move(loadedRig));
		rig = getBySID(id);
		assert(rig != nullptr);


		auto path = mRigFileSystem->getCompiledPath(info.id).path;
		FileSystem::store(path, *rig);
	}

	return rig;
}

nex::Rig::RigInfo nex::AnimationManager::loadRigInfo(const ImportScene& importScene)
{
	auto metaFilePath = absolute(importScene.getFilePath());
	metaFilePath += mMetaExt; // "_meta.ini";

	Configuration meta;
	Rig::RigInfo info;
	meta.addOption("Rig", "id", &info.id, std::string());
	meta.addOption("Rig", "rootBone", &info.rootBone, std::string(""));


	if (!meta.load(metaFilePath.generic_string())) {
		throw_with_trace(nex::ResourceLoadException("nex::AnimationManager::load: meta file couldn't be loaded: " +
			metaFilePath.generic_string()));
	}

	//auto strID = meta.get<std::string>("Rig.id");

	if (info.id == "") {
		throw_with_trace(nex::ResourceLoadException("nex::AnimationManager::load: Rig.id not set in meta file or is empty: " +
			metaFilePath.generic_string()));
	}

	if (info.rootBone == "") {
		throw_with_trace(nex::ResourceLoadException("nex::AnimationManager::load: Rig.rootBone not set in meta file or is empty: " +
			metaFilePath.generic_string()));
	}

	return info;
}

const nex::Rig* nex::AnimationManager::loadRigFromCompiled(const std::string& rigID)
{
	//check if rig is already loaded
	auto sid = SID(rigID);
	auto* storedRig = getBySID(sid);
	if (storedRig) return storedRig;

	//try to load it from compiled rig
	auto path = mRigFileSystem->getCompiledPath(rigID).path;
	auto rig = std::make_unique<Rig>(Rig::createUninitialized());
	auto* rigPtr = rig.get();

	try {
		
		FileSystem::load(path, *rig);
		add(std::move(rig));
	}
	catch (const std::exception & e) {
		LOG(Logger("AnimationManager: "), Error) << e.what();
		throw_with_trace(nex::ResourceLoadException("Couldn't retrieve rig with rig id name: " + rigID));
	}

	return rigPtr;
}

const nex::Rig* nex::AnimationManager::getBySID(unsigned sid) const
{
	auto it = mRigs.find(sid);

	if (it != mRigs.end())
		return it->second.get();

	return nullptr;
}

const nex::BoneAnimation* nex::AnimationManager::loadBoneAnimation(const std::string& name)
{
	const auto sid = SID(name);
	auto* ani = getBoneAnimation(sid);
	//if (ani) return ani;

	auto resolvedPath = mAnimationFileSystem->resolvePath(name);
	auto compiledPath = mAnimationFileSystem->getCompiledPath(resolvedPath).path;

	std::unique_ptr<BoneAnimation> loadedAni;
	Rig::RigInfo info;
	const Rig* rig = nullptr;

	if (!std::filesystem::exists(compiledPath))
	{
		auto importScene = nex::ImportScene::read(resolvedPath, false);
		info = loadRigInfo(importScene);
		rig = getBySID(SID(info.id));

		// try to load it from compiled
		if (!rig) rig = loadRigFromCompiled(info.id);

		if (rig) {
			nex::BoneAnimationLoader animLoader;
			loadedAni = animLoader.load(importScene.getAssimpScene(), rig, name);
			//FileSystem::store(compiledPath, *loadedAni);
		}

		
	}
	else
	{
		loadedAni = std::make_unique<BoneAnimation>(BoneAnimation::createUnintialized());
		FileSystem::load(compiledPath, *loadedAni);

		rig = getBySID(loadedAni->getRigSID());

		// assure that the rig is loaded
		if (!rig) {
			auto id  = loadedAni->getRigID();
			rig = loadRigFromCompiled(id);
			info = rig->getInfo();
		}
	}

	if (!rig) {
		throw_with_trace(nex::ResourceLoadException("nex::AnimationManager::loadBoneAnimation : Rig isn't loaded: " + info.id));
	}

	// add loaded ani to the manager
	ani = loadedAni.get();
	mBoneAnimations.emplace_back(std::move(loadedAni));
	mSidToBoneAnimation.insert({ sid, ani });

	auto& rigAnis = mRigToBoneAnimations.find(rig)->second;
	rigAnis.push_back(ani);

	return ani;
}

const nex::BoneAnimation* nex::AnimationManager::getBoneAnimation(unsigned sid)
{
	auto it = mSidToBoneAnimation.find(sid);
	if (it == mSidToBoneAnimation.end()) return nullptr;

	return it->second;
}

const std::vector<const nex::BoneAnimation*>& nex::AnimationManager::getBoneAnimationsByRig(const Rig* rig)
{
	auto it = mRigToBoneAnimations.find(rig);

	if (it == mRigToBoneAnimations.end()) {
		throw_with_trace(std::runtime_error("nex::AnimationManager::getBoneAnimationsByRig(): Rig is not registered!"));
	}

	return it->second;
}

const nex::Rig* nex::AnimationManager::getRig(const MeshGroup& container) {
	
	for (const auto& mesh : container.getEntries()) {
		auto* skinnedVersion = dynamic_cast<const SkinnedMesh*>(mesh.get());
		if (skinnedVersion) {
			auto rigName = skinnedVersion->getRigID();
			auto* rig = getBySID(SID(rigName));
			if (rig) return rig;

			return loadRigFromCompiled(rigName);
		}
	}

	throw_with_trace(std::invalid_argument("container has no associated rig!"));
	return nullptr;
}

const nex::FileSystem* nex::AnimationManager::getRiggedMeshFileSystem() const
{
	return mRiggedMeshFileSystem.get();
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
	const std::string& compiledRiggedMeshFileExtension,
	const std::string& compiledRigFileExtension,
	const std::string& metaFileExtension)
{
	auto* manager = AnimationManager::get();
	manager->mAnimationFileSystem = std::make_unique<FileSystem>(
		std::vector<std::filesystem::path> {animationRootPath},
		compiledSubFolder,
		compiledAnimationFileExtension
	);

	manager->mRiggedMeshFileSystem = std::make_unique<FileSystem>(
		std::vector<std::filesystem::path> {animationRootPath},
		compiledSubFolder,
		compiledRiggedMeshFileExtension
		);

	manager->mRigFileSystem = std::make_unique<FileSystem>(
		std::vector<std::filesystem::path> {animationRootPath},
		compiledSubFolder,
		compiledRigFileExtension
		);

	manager->mMetaExt = metaFileExtension;
}