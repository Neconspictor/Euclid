#include "ImportScene.hpp"
#include "ImportScene.hpp"
#include <nex/import/ImportScene.hpp>
#include <assimp/postprocess.h>
#include <nex/common/Log.hpp>
#include <nex/util/ExceptionHandling.hpp>
#include <assimp/Importer.hpp>
#include <glm/gtc/type_ptr.hpp>


const std::unordered_set<const aiNode*>& nex::ImportScene::getBones() const
{
	return mBones;
}

std::vector<const aiNode*> nex::ImportScene::getRootBones(const std::unordered_set<const aiNode*>& bones) const
{
	std::vector<const aiNode*> roots;

	for (const auto* bone : bones) {

		// roots are bones that have a parent that isn't a bone itself
		auto it = bones.find(bone->mParent);
		if (it == bones.end()) {
			roots.push_back(bone);
		}
	}

	return roots;
}

const aiNode* nex::ImportScene::getFirstRootBone(bool assertUnique) const
{
	auto bones = getBones();
	auto roots = getRootBones(bones);

	if (assertUnique && roots.size() > 1) {
		throw_with_trace(std::runtime_error("Scene contains more than one root bone!"));
	}

	if (roots.size() == 1) return roots[0];
	return nullptr;
}

std::vector<const aiAnimation*> nex::ImportScene::getAnisForNode(const aiNode* node) const
{
	std::vector<const aiAnimation*> anis;

	for (int i = 0; i < mAssimpScene->mNumAnimations; ++i) {
		auto* ani = mAssimpScene->mAnimations[i];
		if (isKeyFrameAniForNode(ani, node)) {
			anis.push_back(ani);
		}
	}

	return anis;
}

std::vector<const aiAnimation*> nex::ImportScene::getRootAniForNode(const aiNode* node) const
{
	std::vector<const aiAnimation*> anis;
	const auto* parent = node->mParent;

	for (int i = 0; i < mAssimpScene->mNumAnimations; ++i) {
		const auto* ani = mAssimpScene->mAnimations[i];
		const auto isAni = isKeyFrameAniForNode(ani, node);
		const auto IsNotParentAni = !isKeyFrameAniForNode(ani, parent);

		if (isAni && !IsNotParentAni) {
			anis.push_back(ani);
		}
	}

	return anis;
}

std::vector<const aiAnimation*> nex::ImportScene::getBoneAnimations(const std::vector<const aiAnimation*>& keyframeAnis) const
{
	std::vector<const aiAnimation*> boneAnis;
	for (const auto* ani : keyframeAnis) {
		if (isBoneAni(ani)) {
			boneAnis.push_back(ani);
		}
	}

	return boneAnis;
}

std::vector<const aiAnimation*> nex::ImportScene::getKeyFrameAnimations(bool excludeBones) const
{
 std::vector<const aiAnimation*> vec (mAssimpScene->mAnimations, 
									  mAssimpScene->mAnimations + mAssimpScene->mNumAnimations);

 if (excludeBones) {
	 auto boneAnis = getBoneAnimations(vec);

	 // remove bone anis from keyframe ani vector
	 sort(begin(boneAnis), end(boneAnis));
	 vec.erase(remove_if(begin(boneAnis), end(boneAnis),
		 [&](const auto& x) {return binary_search(begin(boneAnis), end(boneAnis), x); }), end(vec));
 }

 return vec;
}

bool nex::ImportScene::isKeyFrameAniForNode(const aiAnimation* ani, const aiNode* node) const
{
	if (!node) return false;

	for (int i = 0; i < ani->mNumChannels; ++i) {
		auto* channel = ani->mChannels[i];
		if (channel->mNodeName == node->mName)
			return true;
	}

	return false;
}

bool nex::ImportScene::isBone(const aiNode* node) const
{
	return mBones.find(node) != mBones.end();
}

bool nex::ImportScene::isBoneAni(const aiAnimation* ani) const
{
	if (!isKeyFrameAni(ani)) return false;

	// Note: isKeyFrameAni checks if there is at least one channel!
	const auto* channel0 = ani->mChannels[0];
	const auto& nodeName = channel0->mNodeName;
	const auto* node = getNode(nodeName);
	
	return isBone(node);
}

bool nex::ImportScene::isKeyFrameAni(const aiAnimation* ani) const
{
	return ani->mNumChannels != 0;
}

nex::ImportScene nex::ImportScene::read(const std::filesystem::path& file, bool doMeshOptimizations) {

	ImportScene importScene;

	unsigned flags = 0;
	if (doMeshOptimizations) {
		flags |= aiProcess_Triangulate
			//| aiProcess_FlipUVs
			| aiProcess_GenSmoothNormals
			| aiProcess_CalcTangentSpace
			| aiProcessPreset_TargetRealtime_MaxQuality;
	}

	const aiScene* scene = importScene.mImporter->ReadFile(file.generic_string(),
		flags);


	if (!scene || scene->mFlags == AI_SCENE_FLAGS_INCOMPLETE && doMeshOptimizations || !scene->mRootNode)
	{
		nex::Logger logger("ImportScene");
		LOG(logger, nex::Error) << "read: " << importScene.mImporter->GetErrorString();
		std::stringstream ss;
		ss << "read: Couldn't load assimp scene: " << file;
		throw_with_trace(std::runtime_error(ss.str()));
	}

	importScene.mAssimpScene = scene;
	importScene.mFile = absolute(file);
	importScene.mDebugSceneNodeRoot = DebugSceneNode::create(scene);

	importScene.collectBones();

	return importScene;
}

nex::ImportScene::ImportScene() : mImporter(std::make_unique<Assimp::Importer>()), mAssimpScene(nullptr)
{
}

nex::ImportScene::~ImportScene() = default;

const std::filesystem::path& nex::ImportScene::getFilePath() const
{
	return mFile;
}

const aiScene* nex::ImportScene::getAssimpScene() const
{
	return mAssimpScene;
}

const aiNode* nex::ImportScene::getNode(const aiString& name) const
{
	std::queue<const aiNode*> nodes;
	nodes.push(mAssimpScene->mRootNode);
	while (!nodes.empty()) {
		auto* node = nodes.front();
		nodes.pop();

		if (strcmp(node->mName.C_Str(), name.C_Str()) == 0) {
			return node;
		}

		for (int i = 0; i < node->mNumChildren; ++i)
			nodes.push(node->mChildren[i]);
	}

	return nullptr;
}

glm::mat4 nex::ImportScene::convert(const aiMatrix4x4& mat)
{
	static_assert(sizeof(glm::mat4) == sizeof(aiMatrix4x4), "size of glm::mat4 doesn't match aiMatrix4x4!");
	return transpose(glm::make_mat4(&mat.a1));
}

glm::vec3 nex::ImportScene::convert(const aiVector3D& vec)
{
	return glm::vec3(vec.x, vec.y, vec.z);
}

glm::quat nex::ImportScene::convert(const aiQuaternion& quat)
{
	return glm::quat(quat.w, quat.x, quat.y, quat.z);
}

bool nex::ImportScene::hasBoneAnimations() const
{
	return mAssimpScene->HasAnimations();
}

bool nex::ImportScene::hasBones() const
{
	if (mAssimpScene->HasMeshes()) {
		for (int i = 0; i < mAssimpScene->mNumMeshes; ++i) {
			auto* mesh = mAssimpScene->mMeshes[i];
			if (mesh->mNumBones > 0)
				return true;
		}
	}
	return false;
}

bool nex::ImportScene::meshDataIsValid() const
{
	return !(mAssimpScene->mFlags & AI_SCENE_FLAGS_INCOMPLETE);
}

void nex::ImportScene::collectBones()
{
	for (int i = 0; i < mAssimpScene->mNumMeshes; ++i) {
		const auto* mesh = mAssimpScene->mMeshes[i];
		for (int j = 0; j < mesh->mNumBones; ++j) {
			const auto& boneName = mesh->mBones[j]->mName;
			const auto* node = getNode(boneName);
			if (node) mBones.insert(node);
		}
	}
}

std::unique_ptr<nex::ImportScene::DebugSceneNode> nex::ImportScene::DebugSceneNode::create(const aiScene* scene)
{
	if (!scene->mRootNode) return nullptr;
	auto root = std::make_unique<DebugSceneNode>();
	root->node = scene->mRootNode;


	auto* currentNode = root.get();

	std::queue<DebugSceneNode*> nodes;
	nodes.push(currentNode);

	while (!nodes.empty()) {
		auto* node = nodes.front();
		nodes.pop();

		node->children.reserve(node->node->mNumChildren);
		for (int i = 0; i < node->node->mNumChildren; ++i) {
			DebugSceneNode child;
			child.node = node->node->mChildren[i];
			node->children.emplace_back(std::move(child));
			nodes.push(&node->children[i]);
		}
	}

	return root;
}