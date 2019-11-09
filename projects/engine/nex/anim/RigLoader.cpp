#include <nex/anim/RigLoader.hpp>
#include <nex/import/ImportScene.hpp>
#include <nex/util/ExceptionHandling.hpp>
#include <nex/util/StringUtils.hpp>

std::unique_ptr<nex::Rig> nex::RigLoader::load(const ImportScene& importScene)
{
	RigData rig;
	const auto* scene = importScene.getAssimpScene();
	std::vector<const aiBone*> bones = getBones(scene);

	// Early exit if there are no bones
	if (bones.size() == 0) return nullptr;

	
	const auto* rootBoneNode = getRootBone(scene, bones);

	
	rig.setRoot(create(getBone(rootBoneNode, bones)));

	static const auto add = [&](const aiNode* node) {
		//check if we have a bone node
		if (isBoneNode(node, bones)) {

			// assure that the parent is a bone node
			if (!isBoneNode(node->mParent, bones)) {
				throw_with_trace(nex::ResourceLoadException(
					"nex::RigLoader::load : Parent of non root bone node has to be bone node!"));
			}

			//ok, insert bone to rig
			std::string parentName = node->mParent->mName.C_Str();
			rig.addBone(create(getBone(node, bones)), parentName);
		}
		return true;
	};

	// add children
	for (int i = 0; i < rootBoneNode->mNumChildren; ++i) {
		for_each(rootBoneNode->mChildren[i], add);
	}

	// TODO : Use better id
	rig.setID(SID(importScene.getFilePath().generic_string()));

	rig.optimize();

	return std::make_unique<Rig>(rig);
}

const aiNode* nex::RigLoader::findByName(const aiScene* scene, const aiString& name) const
{
	const aiNode* result = nullptr;

	static const auto check = [&](const aiNode* node) {
		bool test = node->mName == name;
		if (test) {
			result = node;
		}

		return !test;
	};

	for_each(scene->mRootNode, check);
	
	return result;
}

std::vector<const aiBone*> nex::RigLoader::getBones(const aiScene* scene) const
{
	auto boneCmp = [](const aiBone* a, const aiBone* b) {
		std::string aName = a->mName.C_Str();
		std::string bName = b->mName.C_Str();

		return aName < bName;
	};

	std::set<const aiBone*, decltype(boneCmp)> bones(boneCmp);

	for (int i = 0; i < scene->mNumMeshes; ++i) {
		auto* mesh = scene->mMeshes[i];
		for (int j = 0; j < mesh->mNumBones; ++j) {
			auto result = bones.insert(mesh->mBones[j]);

			// We support only scenes with unique bones
			//if (!result.second) {
			//	throw_with_trace(nex::ResourceLoadException(
			//		"nex::RigLoader::getBones : Bones have to have unique names!"));
			//}
		}
	}

	return std::vector(bones.begin(), bones.end());
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

bool nex::RigLoader::isBoneNode(const aiNode* node, const std::vector<const aiBone*>& bones) const
{
	return getBone(node, bones) != nullptr;
}

std::unique_ptr<nex::BoneData> nex::RigLoader::create(const aiBone* bone) const
{
	if (bone == nullptr) {
		throw_with_trace(nex::ResourceLoadException("nex::RigLoader::create : bone mustn't be null!"));
	}

	std::unique_ptr<BoneData> result = std::make_unique<BoneData>(bone->mName.C_Str());

	static_assert(sizeof(glm::mat4) == sizeof(aiMatrix4x4), "size of glm::mat4 doesn't match aiMatrix4x4!");
	result->setBindPoseTrafo(reinterpret_cast<const glm::mat4&>(bone->mOffsetMatrix));

	/*auto& weights = result->getWeights();

	weights.resize(bone->mNumWeights);
	for (int i = 0; i < bone->mNumWeights; ++i) {
		weights[i] = { bone->mWeights[i].mVertexId, bone->mWeights[i].mWeight };
	}*/

	return result;
}

const aiNode* nex::RigLoader::getRootBone(const aiScene* scene, const std::vector<const aiBone*>& bones) const
{
	//Find all bones with no parent
	std::set<const aiNode*> candidates;
	for (const auto* bone : bones) 
	{
		const auto* node = findByName(scene, bone->mName);

		//assure that the bone is in the hierarchy
		if (node == nullptr) {
			std::string msg = "nex::RigLoader::getRootBone : Bone isn't in node hierarchy: ";
			throw_with_trace(nex::ResourceLoadException(msg + std::string(bone->mName.C_Str())));
		}
		
		//go up the parent hierarchy for finding the root
		const auto* parent = node->mParent;
		const auto* rootBoneNode = node;
		const aiString emptyStr("");
		while (parent) {
			if (parent->mName != emptyStr) {

				// check if the node is indead a bone
				if (isBoneNode(parent, bones)) {
					rootBoneNode = parent;
				}

				// go up
				parent = parent->mParent;
			}
		}

		candidates.insert(rootBoneNode);
	}

	//assert that we only have one root bone
	if (candidates.size() > 1) {
		throw_with_trace(nex::ResourceLoadException("nex::RigLoader::getRootBone : scene contains more than one root bone!"));
	}

	if (candidates.size() == 0) {
		throw_with_trace(nex::ResourceLoadException("nex::RigLoader::getRootBone : No root bone found!"));
	}

	return *candidates.begin();
}