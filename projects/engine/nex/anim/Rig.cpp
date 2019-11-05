#include <nex/anim/Rig.hpp>
#include <nex/util/ExceptionHandling.hpp>
#include <nex/util/StringUtils.hpp>

nex::Bone::Bone(const std::string& name) {
	setName(name);
}


bool nex::Bone::operator==(const Bone& bone)
{
	return bone.mNameHash == mNameHash;
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
	return getByHash(SID(name));
}

nex::Bone* nex::Bone::getByName(const std::string& name)
{
	return getByHash(SID(name));
}

const nex::Bone* nex::Bone::getByHash(unsigned hash) const
{
	if (hasHash(hash)) return this;

	for (const auto& child : mChildren) {
		if (child->hasHash(hash)) return child.get();
	}

	// dig depper into the hierarchy.
	for (const auto& child : mChildren) {
		const auto* bone = child->getByHash(hash);
		if (bone != nullptr) return bone;
	}

	// no suitable bone was found.
	return nullptr;
}

nex::Bone* nex::Bone::getByHash(unsigned hash)
{
	//call const version
	const auto* constThis = static_cast<const Bone*>(this);
	auto* bone = constThis->getByHash(hash);

	//remove constness and return
	return const_cast<Bone*>(bone);
}

unsigned nex::Bone::getHash() const
{
	return mNameHash;
}

const std::string& nex::Bone::getName() const
{
	return mName;
}

bool nex::Bone::hasName(const std::string& name) const
{
	return mNameHash == SID(name);
}

bool nex::Bone::hasHash(unsigned hash) const
{
	return mNameHash == hash;
}

void nex::Bone::setName(const std::string& name)
{
	if (name == "") throw_with_trace(std::invalid_argument("Empty string is not a valid name for a bone"));
	mName = name;
	mNameHash = SID(name);
}

const glm::mat4& nex::Bone::getBoneToParentTrafo() const
{
	return mBoneToParentTrafo;
}

void nex::Bone::setBoneToParentTrafo(const glm::mat4& mat)
{
	mBoneToParentTrafo = mat;
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

const nex::Bone* nex::Rig::getByHash(unsigned hash) const
{
	return mRoot->getByHash(hash);
}

nex::Bone* nex::Rig::getByHash(unsigned hash)
{
	return mRoot->getByHash(hash);
}

void nex::Rig::addBone(std::unique_ptr<Bone> bone, unsigned hash)
{
	auto* parent = mRoot->getByHash(hash);

	if (!parent)
		throw_with_trace(std::invalid_argument("Parent bone isn't present in the hierarchy!"));

	parent->addChild(std::move(bone));
}

void nex::Rig::addBone(std::unique_ptr<Bone> bone, const std::string& parentName)
{
	addBone(std::move(bone), SID(parentName));
}