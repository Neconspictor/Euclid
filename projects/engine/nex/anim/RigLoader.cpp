#include <nex/anim/RigLoader.hpp>
#include <nex/import/ImportScene.hpp>
#include <nex/util/ExceptionHandling.hpp>
#include <nex/util/StringUtils.hpp>

std::unique_ptr<nex::Rig> nex::RigLoader::load(const ImportScene& importScene)
{
	RigData rig;
	auto* scene = importScene.getAssimpScene();

	if (scene->mNumAnimations == 0) {
		throw_with_trace(nex::ResourceLoadException("nex::RigLoader::load : no valid bind pose animation!"));
	}

	auto invRootNodeTrafo = inverse(ImportScene::convert(scene->mRootNode->mTransformation));

	std::vector<const aiNode*> bones = getBones(importScene);

	// Note: Assimp stores only offset matrices for bones with assigned vertices
	std::vector<const aiBone*> aibones = getBonesWithAssignedVertices(importScene);

	// Early exit if there are no bones
	if (bones.size() == 0) return nullptr;

	const auto rootBoneNodes = getRootBones(scene, bones);

	std::vector<std::unique_ptr<BoneData>> rootBoneDatas(rootBoneNodes.size());
	for (int i = 0; i < rootBoneNodes.size(); ++i) {
		rootBoneDatas[i] = create(rootBoneNodes[i], getBone(rootBoneNodes[i], aibones), invRootNodeTrafo);
	}

	rig.setRoots(std::move(rootBoneDatas));

	const auto add = [&](const aiNode* node) {
		std::string parentName = node->mParent->mName.C_Str();
		rig.addBone(create(node, getBone(node, aibones), invRootNodeTrafo), parentName);

		return true;
	};

	// add children
	for (const auto* root : rootBoneNodes) {
		for (int i = 0; i < root->mNumChildren; ++i) {
			for_each(root->mChildren[i], add);
		}
	}

	// TODO : Use better id
	rig.setID(SID(importScene.getFilePath().generic_string()));


	rig.optimize();

	return std::make_unique<Rig>(rig);
}

const aiNode* nex::RigLoader::findByName(const aiScene* scene, const aiString& name) const
{
	const aiNode* result = nullptr;

	const auto check = [&](const aiNode* node) {
		bool test = strcmp(node->mName.C_Str(), name.C_Str()) == 0;
		if (test) {
			result = node;
		}

		return !test;
	};

	for_each(scene->mRootNode, check);
	
	return result;
}

std::vector<const aiNode*> nex::RigLoader::getBones(const ImportScene& importScene) const
{
	static auto boneCmp = [](const aiNode* a, const aiNode* b) {
		std::string aName = a->mName.C_Str();
		std::string bName = b->mName.C_Str();

		return aName < bName;
	};

	// we checked previously in load() that there is at least one animation!
	auto* bindPose = importScene.getAssimpScene()->mAnimations[0];

	std::set<const aiNode*, decltype(boneCmp)> bones(boneCmp);

	for (int i = 0; i < bindPose->mNumChannels; ++i) {
		auto* nodeAnim = bindPose->mChannels[i];
		
		auto* ainode = importScene.getNode(nodeAnim->mNodeName);

		if (ainode == nullptr) {
			throw_with_trace(nex::ResourceLoadException(
				"nex::RigLoader::getBones : Malformed aiScene: Expected node with name: " 
				+ std::string(nodeAnim->mNodeName.C_Str())));
		}

		bones.insert(ainode);		
	}

	return std::vector(bones.begin(), bones.end());
}

std::vector<const aiBone*> nex::RigLoader::getBonesWithAssignedVertices(const ImportScene& importScene) const
{
	static auto boneCmp = [](const aiBone* a, const aiBone* b) {
		std::string aName = a->mName.C_Str();
		std::string bName = b->mName.C_Str();

		return aName < bName;
	};
	std::set<const aiBone*, decltype(boneCmp)> bones(boneCmp);

	auto* scene = importScene.getAssimpScene();
	for (int i = 0; i < scene->mNumMeshes; ++i) {
		auto* mesh = scene->mMeshes[i];
		for (int j = 0; j < mesh->mNumBones; ++j) {
			bones.insert(mesh->mBones[j]);
		}
	}

	return std::vector<const aiBone*>(bones.begin(), bones.end());
}

const aiBone* nex::RigLoader::getBone(const aiNode* node, const std::vector<const aiBone*>& bones) const
{
	const auto& name = node->mName;

	// check if the node is indead a bone
	for (const auto& b : bones) {
		if (b->mName == name) {
			return b;
		}
	}

	return nullptr;
}

bool nex::RigLoader::isBoneWithAssignedVertices(const aiNode* node, const std::vector<const aiBone*>& bones) const
{
	return getBone(node, bones) != nullptr;
}

std::unique_ptr<nex::BoneData> nex::RigLoader::create(const aiNode* boneNode, const aiBone* bone, const glm::mat4& invRootNodeTrafo) const
{
	if (boneNode == nullptr) {
		throw_with_trace(nex::ResourceLoadException("nex::RigLoader::create : bone node mustn't be null!"));
	}

	std::unique_ptr<BoneData> result = std::make_unique<BoneData>(boneNode->mName.C_Str());

	if (bone == nullptr) {
		result->setLocalToBoneSpace(invRootNodeTrafo);
	}
	else {
		auto offset = ImportScene::convert(bone->mOffsetMatrix);
		result->setLocalToBoneSpace(offset * invRootNodeTrafo);
	}

	return result;
}

std::vector<const aiNode*> nex::RigLoader::getRootBones(const aiScene* scene, const std::vector<const aiNode*>& bones) const
{
	// We checked previously in load() that scene has at least one animation!
	const auto* bindPose = scene->mAnimations[0];

	//Find all bone nodes with parents that aren't bones themselves
	std::set<const aiNode*> candidates;
	for (const auto* bone : bones) 
	{

		auto* parent = bone->mParent;

		bool hasParent = parent != nullptr;

		if (hasParent) {
			hasParent = false;
			for (int i = 0; i < bindPose->mNumChannels; ++i) {
				auto* channel = bindPose->mChannels[i];
				auto cmp = strcmp(channel->mNodeName.C_Str(), parent->mName.C_Str()) == 0;
				if (cmp) {
					hasParent = true;
					break;
				}
			}
		}

		if (!hasParent) {
			candidates.insert(bone);
		}
	}

	if (candidates.size() == 0) {
		throw_with_trace(nex::ResourceLoadException("nex::RigLoader::getRootBone : No root bone found!"));
	}

	return std::vector<const aiNode*>(candidates.begin(), candidates.end());
}