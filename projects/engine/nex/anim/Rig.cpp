#include <nex/anim/Rig.hpp>
#include <nex/util/ExceptionHandling.hpp>
#include <nex/util/StringUtils.hpp>

nex::Bone::Bone(const std::string& name) {
	setName(name);
}


bool nex::Bone::operator==(const Bone& bone)
{
	return bone.mNameSID == mNameSID;
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
	return getBySID(SID(name));
}

nex::Bone* nex::Bone::getByName(const std::string& name)
{
	return getBySID(SID(name));
}

const nex::Bone* nex::Bone::getBySID(unsigned sid) const
{
	if (hasSID(sid)) return this;

	for (const auto& child : mChildren) {
		if (child->hasSID(sid)) return child.get();
	}

	// dig depper into the hierarchy.
	for (const auto& child : mChildren) {
		const auto* bone = child->getBySID(sid);
		if (bone != nullptr) return bone;
	}

	// no suitable bone was found.
	return nullptr;
}

nex::Bone* nex::Bone::getBySID(unsigned sid)
{
	//call const version
	const auto* constThis = static_cast<const Bone*>(this);
	auto* bone = constThis->getBySID(sid);

	//remove constness and return
	return const_cast<Bone*>(bone);
}

unsigned nex::Bone::getSID() const
{
	return mNameSID;
}

const std::string& nex::Bone::getName() const
{
	return mName;
}

bool nex::Bone::hasName(const std::string& name) const
{
	return mNameSID == SID(name);
}

bool nex::Bone::hasSID(unsigned sid) const
{
	return mNameSID == sid;
}

void nex::Bone::setName(const std::string& name)
{
	if (name == "") throw_with_trace(std::invalid_argument("Empty string is not a valid name for a bone"));
	mName = name;
	mNameSID = SID(name);
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

nex::Rig::Rig(std::unique_ptr<Bone> root) : mRoot(std::move(root))
{
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

const nex::Bone* nex::Rig::getBySID(unsigned sid) const
{
	return mRoot->getBySID(sid);
}

nex::Bone* nex::Rig::getBySID(unsigned sid)
{
	return mRoot->getBySID(sid);
}

void nex::Rig::addBone(std::unique_ptr<Bone> bone, unsigned sid)
{
	auto* parent = mRoot->getBySID(sid);

	if (!parent)
		throw_with_trace(std::invalid_argument("Parent bone isn't present in the hierarchy!"));

	parent->addChild(std::move(bone));
}

void nex::Rig::addBone(std::unique_ptr<Bone> bone, const std::string& parentName)
{
	addBone(std::move(bone), SID(parentName));
}