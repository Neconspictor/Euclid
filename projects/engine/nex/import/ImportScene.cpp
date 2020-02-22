#include "ImportScene.hpp"
#include "ImportScene.hpp"
#include <nex/import/ImportScene.hpp>
#include <assimp/postprocess.h>
#include <nex/common/Log.hpp>
#include <nex/util/ExceptionHandling.hpp>
#include <assimp/Importer.hpp>
#include <glm/gtc/type_ptr.hpp>


std::unordered_set<const aiNode*> nex::ImportScene::collectBones() const
{
	std::unordered_set<const aiNode*> nodes;

	for (int i = 0; i < mAssimpScene->mNumMeshes; ++i) {
		const auto* mesh = mAssimpScene->mMeshes[i];
		for (int j = 0; j < mesh->mNumBones; ++j) {
			const auto& boneName = mesh->mBones[j]->mName;
			const auto* node = getNode(boneName);
			if (node) nodes.insert(node);
		}
	}
	return nodes;
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
	auto bones = collectBones();
	auto roots = getRootBones(bones);

	if (assertUnique && roots.size() > 1) {
		throw_with_trace(std::runtime_error("Scene contains more than one root bone!"));
	}

	if (roots.size() == 1) return roots[0];
	return nullptr;
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
	importScene.mFile = file;
	importScene.mDebugSceneNodeRoot = DebugSceneNode::create(scene);

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
