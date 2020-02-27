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
#include <nex/anim/AnimationLoader.hpp>

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

	//get rig candidates
	auto* root = importScene.getFirstRootBone();
	if (!root) return nullptr;

	const std::string rigID = root->mName.C_Str();
	const auto sid = SID(rigID);

	auto* rig = getBySID(sid);
	if (rig == nullptr) {
		RigLoader loader;
		auto loadedRig = loader.load(importScene, rigID);
		add(std::move(loadedRig));
		rig = getBySID(sid);
		assert(rig != nullptr);


		auto path = mRigFileSystem->getCompiledPath(rigID).path;
		FileSystem::store(path, *rig);
	}

	return rig;
}

const nex::Rig* nex::AnimationManager::load(const std::string& rigID)
{
	auto* rig = getBySID(SID(rigID));
	if (!rig) {
		rig = loadRigFromCompiled(rigID);
	}
	return rig;
}

const nex::Rig* nex::AnimationManager::load(const ImportScene& importScene, const aiNode* root)
{
	const std::string rigID = root->mName.C_Str();
	const auto sid = SID(rigID);

	auto* rig = getBySID(sid);
	if (rig == nullptr) {
		RigLoader loader;
		auto loadedRig = loader.load(importScene, rigID);
		add(std::move(loadedRig));
		rig = getBySID(sid);
		assert(rig != nullptr);


		auto path = mRigFileSystem->getCompiledPath(rigID).path;
		FileSystem::store(path, *rig);
	}

	return rig;
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

std::unique_ptr<nex::BoneAnimation> nex::AnimationManager::loadSingleBoneAnimation(const aiAnimation* aiBoneAni, const ImportScene& importScene)
{
	std::unique_ptr<BoneAnimation> result;

	// generate a unique name for the animation
	const std::string name = generateUniqueKeyFrameAniName(aiBoneAni, importScene);

	const auto& roots = importScene.getRootBones(importScene.getBones());

	const aiNode* root = nullptr;

	for (const auto* r : roots) {
		if (importScene.isKeyFrameAniForNode(aiBoneAni, r)) {
			root = r;
			break;
		}
	}

	if (!root) throw_with_trace(std::runtime_error("Animation file contains no rig: " + importScene.getFilePath().generic_u8string()));

	const auto rigID = std::string(root->mName.C_Str());
	const auto* rig = getBySID(SID(rigID));

	// try to load it from compiled
	if (!rig) rig = loadRigFromCompiled(rigID);
	if (!rig) throw_with_trace(std::runtime_error("Couldn't load rig with id " + rigID));

	nex::BoneAnimationLoader animLoader;
	auto loadedBoneAni = animLoader.load(aiBoneAni, rig, name);

	return std::make_unique<BoneAnimation>(std::move(loadedBoneAni));
}

std::unique_ptr<nex::KeyFrameAnimation> nex::AnimationManager::loadSingleKeyFrameAnimation(const aiAnimation* aiKeyFrameAni, const ImportScene& importScene)
{
	std::unique_ptr<KeyFrameAnimation> result;

	// generate a unique name for the animation
	const std::string name = generateUniqueKeyFrameAniName(aiKeyFrameAni, importScene);

	nex::KeyFrameAnimationLoader animLoader;
	auto loadeAni = animLoader.load(aiKeyFrameAni, name, KeyFrameAnimation::ChannelIDGenerator());

	return std::make_unique<KeyFrameAnimation>(std::move(loadeAni));
}

std::string nex::AnimationManager::generateUniqueKeyFrameAniName(const aiAnimation* aiKeyFrameAni, const ImportScene& importScene)
{
	const auto* scene = importScene.getAssimpScene();

	auto index = getKeyFrameAniIndex(aiKeyFrameAni, scene);

	return importScene.getFilePath().generic_u8string() + "#" + std::to_string(index) + "#" + std::string(aiKeyFrameAni->mName.C_Str());
}

unsigned nex::AnimationManager::getKeyFrameAniIndex(const aiAnimation* aiKeyFrameAni, const aiScene* scene)
{
	for (int i = 0; i < scene->mNumAnimations; ++i) {
		if (aiKeyFrameAni == scene->mAnimations[i]) {
			return i;
		}
	}

	throw_with_trace(std::invalid_argument("animation is no registered keyframe animation!"));
	return 0;
}

const nex::Rig* nex::AnimationManager::getBySID(unsigned sid) const
{
	auto it = mRigs.find(sid);

	if (it != mRigs.end())
		return it->second.get();

	return nullptr;
}

std::vector<const nex::BoneAnimation*> nex::AnimationManager::loadBoneAnimations(const std::filesystem::path& filePath)
{
	//const auto sid = SID(name);
	//auto* ani = getBoneAnimation(sid);
	//if (ani) return ani;

	auto resolvedPath = mAnimationFileSystem->resolvePath(filePath);
	auto compiledPath = mAnimationFileSystem->getCompiledPath(resolvedPath).path;

	std::vector<std::unique_ptr<BoneAnimation>> boneAnis;

	if (!std::filesystem::exists(compiledPath))
	{
		auto importScene = nex::ImportScene::read(resolvedPath, false);
		if (!importScene.hasBoneAnimations()) {
			throw_with_trace(std::logic_error("Specified file contains no bone animation: " + compiledPath.generic_string()));
		}

		auto aiBoneAnis = importScene.getBoneAnimations(importScene.getKeyFrameAnimations());

		for (const auto* aiAni : aiBoneAnis) {
			auto ani = loadSingleBoneAnimation(aiAni, importScene);
			boneAnis.push_back(std::move(ani));
		}
	}
	else
	{

		std::vector<BoneAnimation> storeVec;

		FileSystem::load(compiledPath, storeVec);

		for (auto& ani : storeVec) {
			boneAnis.emplace_back(std::make_unique<BoneAnimation>(std::move(ani)));
		}

		storeVec.clear();

		//TODO rigs
	}

	// add loaded anis to the manager
	std::vector<const BoneAnimation*> result;

	while (!boneAnis.empty()) {

		auto ani = std::move(boneAnis.back());
		boneAnis.pop_back();
		const auto sid = SID(ani->getName());
		const auto* aniPtr = ani.get();

		mBoneAnimations.insert({sid, std::move(ani)});
		mSidToBoneAnimation.insert({ sid, aniPtr });
		auto& rigAnis = mRigToBoneAnimations.find(aniPtr->getRig())->second;
		rigAnis.insert(aniPtr);

		// Finally we can savely add the ani to the result vector
		result.push_back(aniPtr);
	}

	return result;
}

std::vector<std::unique_ptr<nex::KeyFrameAnimation>> nex::AnimationManager::loadKeyFrameAnimations(const std::filesystem::path& filePath)
{
	auto resolvedPath = mAnimationFileSystem->resolvePath(filePath);
	auto compiledPath = mAnimationFileSystem->getCompiledPath(resolvedPath).path;

	std::vector<std::unique_ptr<KeyFrameAnimation>> keyFrameAnis;

	if (!std::filesystem::exists(compiledPath))
	{
		auto importScene = nex::ImportScene::read(resolvedPath, false);
		if (!importScene.hasBoneAnimations()) {
			throw_with_trace(std::logic_error("Specified file contains no keyframe animations: " + compiledPath.generic_string()));
		}

		auto aiKeyFrameAnis = importScene.getKeyFrameAnimations(true);

		for (const auto* aiAni : aiKeyFrameAnis) {
			auto ani = loadSingleBoneAnimation(aiAni, importScene);
			keyFrameAnis.push_back(std::move(ani));
		}
	}
	else
	{
		std::vector<KeyFrameAnimation> storeVec;

		FileSystem::load(compiledPath, storeVec);

		for (auto& ani : storeVec) {
			keyFrameAnis.emplace_back(std::make_unique<KeyFrameAnimation>(std::move(ani)));
		}

		storeVec.clear();
	}

	return keyFrameAnis;
}

const nex::BoneAnimation* nex::AnimationManager::getBoneAnimation(unsigned sid)
{
	auto it = mSidToBoneAnimation.find(sid);
	if (it == mSidToBoneAnimation.end()) return nullptr;

	return it->second;
}

const std::set<const nex::BoneAnimation*>& nex::AnimationManager::getBoneAnimationsByRig(const Rig* rig)
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

nex::AnimationManager* nex::AnimationManager::get()
{
	static AnimationManager instance;
	return &instance;
}

void nex::AnimationManager::init(
	const std::filesystem::path& resourceRootPath,
	const std::string& compiledResourcePath,
	const std::string& compiledAnimationFileExtension,
	const std::string& compiledRiggedMeshFileExtension,
	const std::string& compiledRigFileExtension,
	const std::string& metaFileExtension)
{
	auto* manager = AnimationManager::get();
	manager->mAnimationFileSystem = std::make_unique<FileSystem>(
		std::vector<std::filesystem::path> {resourceRootPath},
		compiledResourcePath,
		compiledAnimationFileExtension
	);

	std::filesystem::path compiledRigPath = compiledResourcePath;
	compiledRigPath /= "rigs/";

	manager->mRigFileSystem = std::make_unique<FileSystem>(
		std::vector<std::filesystem::path> {resourceRootPath},
		compiledRigPath,
		compiledRigFileExtension
		);

	manager->mMetaExt = metaFileExtension;
}