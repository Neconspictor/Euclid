#ifndef GLM_ENABLE_EXPERIMENTAL
#define GLM_ENABLE_EXPERIMENTAL
#endif

#include <nex/anim/BoneAnimation.hpp>
#include <nex/anim/Rig.hpp>
#include <nex/util/ExceptionHandling.hpp>
#include <algorithm>
#include <nex/math/Math.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>
#include <nex/anim/AnimationManager.hpp>
#include <functional>

void nex::BoneAnimationData::setRig(const Rig* rig)
{
	mRig = rig;
}

nex::BoneAnimation::BoneAnimation(const BoneAnimationData& data) : KeyFrameAnimation(data)
{

	if (data.mRig == nullptr) throw_with_trace(std::invalid_argument("nex::BoneAnimation : rig mustn't be null!"));

	mRigID = data.mRig->getID();
	mRigSID = data.mRig->getSID();

	auto* rig = getRig();

	if (rig == nullptr) throw_with_trace(std::runtime_error("nex::BoneAnimation : rig from rig sid mustn't be null! Fix that bug!"));

	mChannelCount = static_cast<unsigned>(rig->getBones().size());

	struct BoneChannelIDGenerator : public ChannelIDGenerator {

		BoneChannelIDGenerator(const BoneAnimationData& data) : data(data) {}

		short operator()(unsigned keyFrameSID) const override {
			const auto* bone = data.mRig->getBySID(keyFrameSID);
			return bone->getID();
		}

		const BoneAnimationData& data;
	};

	init(data, BoneChannelIDGenerator(data));
}

void nex::BoneAnimation::applyParentHierarchyTrafos(std::vector<glm::mat4>& vec) const
{
	auto* rig = getRig();
	const auto& bones = rig->getBones();
	auto invRootTrafo = rig->getInverseRootTrafo();
	auto rootTrafo = inverse(invRootTrafo);// *glm::mat4(0.03f);
	//invRootTrafo = inverse(rootTrafo);

	if (vec.size() != rig->getBones().size()) {
		throw_with_trace(std::invalid_argument(
			"nex::BoneAnimation::applyParentHierarchyTrafos : Matrix vector argument has to have the same size like there are bones!"));
	}

	const std::function<void(const Bone*, const glm::mat4&)> recursive = [&](const Bone* bone, const glm::mat4& parentTrafo) {

		auto id = bone->getID();
		const auto& nodeTrafo = vec[id];
		const auto& offset = bone->getOffsetMatrix();

		auto trafo = parentTrafo * nodeTrafo;
		vec[id] = invRootTrafo * trafo * offset;
		
		// recursive propagation
		const auto& children = bone->getChildrenIDs();
		for (int i = 0; i < bone->getChildrenCount(); ++i) {
			const auto childID = children[i];
			recursive(&bones[childID], trafo);
		}
	};

	recursive(rig->getRoot(), rootTrafo);// glm::mat4(1.0f));
}

const nex::Rig* nex::BoneAnimation::getRig() const
{
	return nex::AnimationManager::get()->getBySID(mRigSID);
}

const std::string& nex::BoneAnimation::getRigID() const
{
	return mRigID;
}

unsigned nex::BoneAnimation::getRigSID() const
{
	return mRigSID;
}

void nex::BoneAnimation::write(nex::BinStream& out) const
{
	KeyFrameAnimation::write(out);
	out << mRigID;
	out << mRigSID;
}

void nex::BoneAnimation::load(nex::BinStream& in)
{
	KeyFrameAnimation::load(in);
	in >> mRigID;
	in >> mRigSID;
}

nex::BinStream& nex::operator>>(nex::BinStream& in, BoneAnimation& ani)
{
	ani.load(in);
	return in;
}

nex::BinStream& nex::operator<<(nex::BinStream& out, const BoneAnimation& ani)
{
	ani.write(out);
	return out;
}