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

	bone->setParent(this);
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

const std::string& nex::BoneData::getName() const
{
	return mName;
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
	mName = name;
}

const glm::mat4& nex::BoneData::getLocalToBoneSpace() const
{
	return mLocalToBoneSpace;
}

void nex::BoneData::setLocalToBoneSpace(const glm::mat4& mat)
{
	mLocalToBoneSpace = mat;
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

nex::RigData::RigData()
{
	recalculateBoneCount();
}

unsigned nex::RigData::getBoneCount() const
{
	return mBoneCount;
}

unsigned nex::RigData::getID() const
{
	return mID;
}

const std::vector<std::unique_ptr<nex::BoneData>>& nex::RigData::getRoots() const
{
	return mRoots;
}

std::vector<std::unique_ptr<nex::BoneData>>& nex::RigData::getRoots()
{
	return mRoots;
}

const nex::BoneData* nex::RigData::getByName(const std::string& name) const
{
	for (const auto& root : mRoots) {
		auto* result = root->getByName(name);
		if (result) return result;
	}
	return nullptr;
}

nex::BoneData* nex::RigData::getByName(const std::string& name)
{
	const RigData* constThis = this;
	return const_cast<nex::BoneData*> (constThis->getByName(name));
}

const nex::BoneData* nex::RigData::getBySID(unsigned sid) const
{

	for (const auto& root : mRoots) {
		auto* result = root->getBySID(sid);
		if (result) return result;
	}
	return nullptr;
}

nex::BoneData* nex::RigData::getBySID(unsigned sid)
{
	const RigData* constThis = this;
	return const_cast<nex::BoneData*> (constThis->getBySID(sid));
}

void nex::RigData::addBone(std::unique_ptr<BoneData> bone, unsigned sid)
{
	auto* parent = getBySID(sid);

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

	const auto assignID = [&](BoneData* bone) {
		bone->optimize(id);
		++id;
		return true;
	};

	for (auto& root : mRoots) {
		root->for_each(assignID);
	}

	assert(mBoneCount == id);
	mOptimized = true;
}

void nex::RigData::setID(unsigned id)
{
	mID = id;
}

void nex::RigData::setRoots(std::vector<std::unique_ptr<BoneData>> roots)
{
	mRoots = std::move(roots);
	recalculateBoneCount();
}

void nex::RigData::recalculateBoneCount()
{
	mBoneCount = 0;

	auto advance = [&](BoneData*) {
		++mBoneCount; 
		return true; 
	};

	for (auto& root : mRoots) {
		root->for_each(advance);
	}
}

nex::Bone::Bone(const BoneData& bone)
{
	if (!bone.isOptimized()) {
		throw_with_trace(nex::ResourceLoadException("nex::Bone::Bone : BoneData argument has to be optimized!"));
	}


	mBoneID = bone.getBoneID();
	mLocalToBoneSpace = bone.getLocalToBoneSpace();

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

const glm::mat4& nex::Bone::getLocalToBoneSpace() const
{
	return mLocalToBoneSpace;
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

	mID = data.getID();

	mBones.resize(data.getBoneCount());
	mSIDs.resize(data.getBoneCount());
	mSidToBoneId.reserve(data.getBoneCount());

	const auto fill = [&](const BoneData* bone) {
		const auto id = bone->getBoneID();
		mBones[id] = Bone(*bone);
		mSIDs[id] = bone->getSID();
		mSidToBoneId.insert(std::pair<unsigned, short>(bone->getSID(), id));
		return true;
	};

	mRoots.reserve(data.getRoots().size());

	for (auto& root : data.getRoots()) {
		root->for_each(fill);
		mRoots.push_back(&mBones[root->getBoneID()]);
	}
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

unsigned nex::Rig::getID() const
{
	return mID;
}

const std::vector<const nex::Bone*>& nex::Rig::getRoots() const
{
	return mRoots;
}