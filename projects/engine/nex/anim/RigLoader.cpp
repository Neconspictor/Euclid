#include <nex/anim/RigLoader.hpp>
#include <nex/import/ImportScene.hpp>
#include <nex/util/ExceptionHandling.hpp>
#include <nex/util/StringUtils.hpp>

std::unique_ptr<nex::Rig> nex::RigLoader::load(const ImportScene& importScene, const std::string& id)
{
	RigData rig;

	if (!importScene.meshDataIsValid()) {
		throw_with_trace(nex::ResourceLoadException("nex::RigLoader::load : Cannot create Rig from invalid mesh in scene!"));
	}


	auto* scene = importScene.getAssimpScene();

	if (scene->mNumAnimations == 0) {
		throw_with_trace(nex::ResourceLoadException("nex::RigLoader::load : no valid bind pose animation!"));
	}


	std::unordered_set<aiNode*> testNodes;
	std::queue<aiNode*> queue;
	queue.push(scene->mRootNode);
	while (!queue.empty()) {
		auto* node = queue.front();
		queue.pop();

		testNodes.insert(node);
		for (int i = 0; i < node->mNumChildren; ++i) {
			queue.push(node->mChildren[i]);
		}
	}


	auto invRootNodeTrafo = inverse(ImportScene::convert(scene->mRootNode->mTransformation));
	rig.setInverseRootTrafo(invRootNodeTrafo);

	std::vector<const aiNode*> bones = getBones(importScene);

	// Note: Assimp stores only offset matrices for bones with assigned vertices
	std::vector<const aiBone*> aibones = getBonesWithAssignedVertices(importScene);

	// Early exit if there are no bones
	if (bones.size() == 0) return nullptr;

	const auto* rootBoneNode = getRootBone(scene, bones);
	auto rootBone = create(rootBoneNode, getBone(rootBoneNode, aibones));
	rig.setRoot(std::move(rootBone));

	const auto add = [&](const aiNode* node) {
		std::string parentName = node->mParent->mName.C_Str();
		rig.addBone(create(node, getBone(node, aibones)), parentName);

		return true;
	};

	// add children

		for (int i = 0; i < rootBoneNode->mNumChildren; ++i) {
			for_each(rootBoneNode->mChildren[i], add);
		}

	rig.setID(id);


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

		//if (aName != bName) return aName < bName;

		//return a < b;
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

std::unique_ptr<nex::BoneData> nex::RigLoader::create(const aiNode* boneNode, const aiBone* bone) const
{
	if (boneNode == nullptr) {
		throw_with_trace(nex::ResourceLoadException("nex::RigLoader::create : bone node mustn't be null!"));
	}

	std::unique_ptr<BoneData> result = std::make_unique<BoneData>(boneNode->mName.C_Str());

	glm::mat4 offset(1.0f);

	if (bone != nullptr) {
		offset = ImportScene::convert(bone->mOffsetMatrix);
	}

	auto trafo = ImportScene::convert(boneNode->mTransformation);
	result->setLocalToBoneSpace(offset);
	int i = 0;

	return result;
}

const aiNode* nex::RigLoader::getRootBone(const aiScene* scene, const std::vector<const aiNode*>& bones) const
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

	if (candidates.size() > 1) {
		throw_with_trace(nex::ResourceLoadException("nex::RigLoader::getRootBone : Expected only one root bone!"));
	}

	return *candidates.begin();
}