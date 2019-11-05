#include <nex/anim/Rig.hpp>
#include <nex/util/ExceptionHandling.hpp>

bool nex::Bone::operator==(const Bone& bone)
{
	return bone.mName == mName;
}

void nex::Bone::addChild(std::unique_ptr<Bone> bone)
{
	constexpr auto* errorMsg = "Equal bone already exists in hierarchy";

	if (*this == *bone) throw_with_trace(std::invalid_argument(errorMsg));

	for (const auto& child : mChildren) {
		if (*child == *bone) throw_with_trace(std::invalid_argument(errorMsg));
	}	

	mChildren.emplace_back(std::move(bone));
}

const nex::Bone* nex::Bone::getByName(const std::string& name) const
{
	if (hasName(name)) return this;

	for (const auto& child : mChildren) {
		if (child->hasName(name)) return child.get();
	}

	// dig depper into the hierarchy.
	for (const auto& child : mChildren) {
		const auto* bone = child->getByName(name);
		if (bone != nullptr) return bone;
	}

	// no suitable bone was found.
	return nullptr;
}

nex::Bone* nex::Bone::getByName(const std::string& name)
{
	//call const version
	const auto* constThis = static_cast<const Bone*>(this);
	auto* bone = constThis->getByName(name);

	//remove constness and return
	return const_cast<Bone*>(bone);
}

const std::string& nex::Bone::getName() const
{
	return mName;
}

bool nex::Bone::hasName(const std::string& name) const
{
	return mName == name;
}

void nex::Bone::setName(const std::string& name)
{
	mName = name;
}

const glm::mat4& nex::Bone::getBoneToParentTrafo() const
{
	return mBoneToParentTrafo;
}

void nex::Bone::setBoneToParentTrafo(const glm::mat4& mat)
{
	mBoneToParentTrafo = mat;
}

bool nex::Bone::isInHierarchy(const Bone* bone) const
{
	if (bone == nullptr) return false;
	if (this == bone) return true;

	for (auto& child : mChildren) 
	{
		if (child->isInHierarchy(bone))
			return true;
	}

	return false;
}

bool nex::Bone::isRoot() const
{
	return mParent == nullptr;
}

const nex::Bone* nex::Bone::getParent() const
{
	return mParent;
}

nex::Bone* nex::Bone::getParent()
{
	return mParent;
}

void nex::Bone::setParent(Bone* parent)
{
	mParent = parent;
}

const nex::Bone::BoneVec& nex::Bone::getChildren() const
{
	return mChildren;
}

nex::Bone::BoneVec& nex::Bone::getChildren()
{
	return mChildren;
}

const nex::Bone* nex::Rig::getRoot() const
{
	return mRoot.get();
}

nex::Bone* nex::Rig::getRoot()
{
	return mRoot.get();
}

const nex::Bone* nex::Rig::getByName(const std::string& name) const
{
	return mRoot->getByName(name);
}

nex::Bone* nex::Rig::getByName(const std::string& name)
{
	return mRoot->getByName(name);
}

bool nex::Rig::isInHierarchy(const Bone* bone) const
{
	return mRoot->isInHierarchy(bone);
}

void nex::Rig::addBone(std::unique_ptr<Bone> bone, Bone* parent)
{
	if (!isInHierarchy(parent))
		throw_with_trace(std::invalid_argument("Parent bone isn't present in the hierarchy!"));

	parent->addChild(std::move(bone));
}

void nex::Rig::addBone(std::unique_ptr<Bone> bone, const std::string& parentName)
{
	auto* parent = getByName(parentName);
	addBone(std::move(bone), parent);
}