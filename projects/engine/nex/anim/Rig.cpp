#include <nex/anim/Rig.hpp>
#include <nex/util/ExceptionHandling.hpp>
#include <nex/util/StringUtils.hpp>
#include <nex/exception/ResourceLoadException.hpp>

nex::BoneData::BoneData(const std::string& name) {
	setName(name);
}


bool nex::BoneData::operator==(const BoneData& bone)
{
	return bone.mNameSID == mNameSID;
}

void nex::BoneData::addChild(std::unique_ptr<BoneData> bone)
{
	constexpr auto* errorMsg = "Equal bone already exists in hierarchy";

	if (*this == *bone) throw_with_trace(std::invalid_argument(errorMsg));

	for (const auto& child : mChildren) {
		if (*child == *bone) throw_with_trace(std::invalid_argument(errorMsg));
	}	

	mChildren.emplace_back(std::move(bone));
}

const nex::BoneData* nex::BoneData::getByName(const std::string& name) const
{
	return getBySID(SID(name));
}

nex::BoneData* nex::BoneData::getByName(const std::string& name)
{
	return getBySID(SID(name));
}

const nex::BoneData* nex::BoneData::getBySID(unsigned sid) const
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

nex::BoneData* nex::BoneData::getBySID(unsigned sid)
{
	//call const version
	const auto* constThis = static_cast<const BoneData*>(this);
	auto* bone = constThis->getBySID(sid);

	//remove constness and return
	return const_cast<BoneData*>(bone);
}

unsigned nex::BoneData::getSID() const
{
	return mNameSID;
}

bool nex::BoneData::hasName(const std::string& name) const
{
	return mNameSID == SID(name);
}

bool nex::BoneData::hasSID(unsigned sid) const
{
	return mNameSID == sid;
}

void nex::BoneData::setName(const std::string& name)
{
	if (name == "") throw_with_trace(std::invalid_argument("Empty string is not a valid name for a bone"));
	mNameSID = SID(name);
}

const glm::mat4& nex::BoneData::getBindPoseTrafo() const
{
	return mBindPoseTrafo;
}

void nex::BoneData::setBindPoseTrafo(const glm::mat4& mat)
{
	mBindPoseTrafo = mat;
}

bool nex::BoneData::isOptimized() const
{
	return mOptimized;
}

bool nex::BoneData::isRoot() const
{
	return mParent == nullptr;
}

const nex::BoneData* nex::BoneData::getParent() const
{
	return mParent;
}

nex::BoneData* nex::BoneData::getParent()
{
	return mParent;
}

void nex::BoneData::setParent(BoneData* parent)
{
	mParent = parent;
}

const nex::BoneData::BoneVec& nex::BoneData::getChildren() const
{
	return mChildren;
}

nex::BoneData::BoneVec& nex::BoneData::getChildren()
{
	return mChildren;
}

short nex::BoneData::getBoneID() const
{
	return mBoneID;
}

void nex::BoneData::optimize(short id)
{
	mBoneID = id;
	mOptimized = true;
}


/*const std::vector<nex::Weight>& nex::Bone::getWeights() const
{
	return mWeights;
}

std::vector<nex::Weight>& nex::Bone::getWeights()
{
	return mWeights;
}*/

nex::RigData::RigData() : mRoot(nullptr)
{
	recalculateBoneCount();
}

unsigned nex::RigData::getBoneCount() const
{
	return mBoneCount;
}

const nex::BoneData* nex::RigData::getRoot() const
{
	return mRoot.get();
}

nex::BoneData* nex::RigData::getRoot()
{
	return mRoot.get();
}

const nex::BoneData* nex::RigData::getByName(const std::string& name) const
{
	return mRoot->getByName(name);
}

nex::BoneData* nex::RigData::getByName(const std::string& name)
{
	return mRoot->getByName(name);
}

const nex::BoneData* nex::RigData::getBySID(unsigned sid) const
{
	return mRoot->getBySID(sid);
}

nex::BoneData* nex::RigData::getBySID(unsigned sid)
{
	return mRoot->getBySID(sid);
}

void nex::RigData::addBone(std::unique_ptr<BoneData> bone, unsigned sid)
{
	auto* parent = mRoot->getBySID(sid);

	if (!parent)
		throw_with_trace(std::invalid_argument("Parent bone isn't present in the hierarchy!"));

	parent->addChild(std::move(bone));

	// update bone count
	++mBoneCount;
}

void nex::RigData::addBone(std::unique_ptr<BoneData> bone, const std::string& parentName)
{
	addBone(std::move(bone), SID(parentName));
}

bool nex::RigData::isOptimized() const
{
	return mOptimized;
}

void nex::RigData::optimize()
{
	//Each bone gets an index. The highest index is the "bone count minus one".
	unsigned id = 0;

	static const auto assignID = [&](BoneData* bone) {
		bone->optimize(id);
		++id;
		return true;
	};

	mRoot->for_each(assignID);
	assert(mBoneCount == id);
	mOptimized = true;
}

void nex::RigData::setRoot(std::unique_ptr<BoneData> bone)
{
	mRoot = std::move(bone);
	recalculateBoneCount();
}

void nex::RigData::recalculateBoneCount()
{
	mBoneCount = 0;
	if (!mRoot) return;

	auto advance = [&](BoneData*) {
		++mBoneCount; 
		return true; 
	};
	mRoot->for_each(advance);
}

nex::Bone::Bone(const BoneData& bone)
{
	if (!bone.isOptimized()) {
		throw_with_trace(nex::ResourceLoadException("nex::Bone::Bone : BoneData argument has to be optimized!"));
	}


	mBoneID = bone.getBoneID();
	mBindPoseTrafo = bone.getBindPoseTrafo();
	//mNameSID = bone.getSID();
	const auto* parent = bone.getParent();
	if (parent) {
		mParentID = parent->getBoneID();
	}
	else {
		mParentID = -1;
	}

	const auto& children = bone.getChildren();
	if (children.size() > Bone::MAX_CHILDREN_SIZE) {
		throw_with_trace(nex::ResourceLoadException("nex::Bone::Bone : Too many bone children!"));
	}

	mChildrenCount = 0;
	for (const auto& child : children) {
		mChildrenIDs[mChildrenCount] = child->getBoneID();
		++mChildrenCount;
	}
}

const glm::mat4& nex::Bone::getBindPoseTrafo() const
{
	return mBindPoseTrafo;
}

unsigned short nex::Bone::getChildrenCount() const
{
	return mChildrenCount;
}

const std::array<short, nex::Bone::MAX_CHILDREN_SIZE>& nex::Bone::getChildrenIDs() const
{
	return mChildrenIDs;
}

short nex::Bone::getID() const
{
	return mBoneID;
}

/*unsigned nex::Bone::getNameSID() const
{
	return mNameSID;
}*/

short nex::Bone::getParentID() const
{
	return mParentID;
}

nex::Rig::Rig(const RigData& data)
{
	if (!data.isOptimized()) {
		throw_with_trace(nex::ResourceLoadException("nex::Rig::Rig : RigData argument has to be optimized!"));
	}

	mBones.resize(data.getBoneCount());
	mSIDs.resize(data.getBoneCount());
	mSidToBoneId.reserve(data.getBoneCount());

	static const auto fill = [&](const BoneData* bone) {
		const auto id = bone->getBoneID();
		mBones[id] = Bone(*bone);
		mSIDs[id] = bone->getSID();
		mSidToBoneId.insert(std::pair<unsigned, short>(bone->getSID(), id));
		return true;
	};

	data.getRoot()->for_each(fill);
}

const std::vector<nex::Bone>& nex::Rig::getBones() const
{
	return mBones;
}

const std::vector<unsigned> nex::Rig::getSIDs() const
{
	return mSIDs;
}

const nex::Bone* nex::Rig::getByName(const std::string& name) const
{
	return getBySID(SID(name));
}

const nex::Bone* nex::Rig::getBySID(unsigned sid) const
{
	auto it = mSidToBoneId.find(sid);
	if (it != mSidToBoneId.end()) {
		auto id = it->second;
		return &mBones[id];
	}
	return nullptr;
}