#include <nex/scene/VobBluePrint.hpp>

nex::VobBluePrint::VobBluePrint(std::unique_ptr<Vob> bluePrint) : mVob(std::move(bluePrint))
{
	if (!mVob) {
		throw_with_trace(std::invalid_argument("blue-print vob mustn't be null!"));
	}

	auto size = fillMap(*mVob, 0) + 1;
	assert(size == mBluePrintChildVobNameSIDToMatrixIndex.size());

	mBluePrintRootNameSID = SID(mVob->getName());
}

std::unique_ptr<nex::Vob> nex::VobBluePrint::createBluePrint() const
{
	auto copy = mVob->createBluePrintCopy();

	copy->setBluePrint(this);

	return copy;
}

std::unique_ptr<nex::KeyFrameAnimation::ChannelIDGenerator> nex::VobBluePrint::createGenerator() const
{
	struct Generator : public nex::KeyFrameAnimation::ChannelIDGenerator {

		Generator(const VobBluePrint* bluePrint) : mBluePrint(bluePrint) {}

		nex::ChannelID operator()(nex::Sid keyFrameSID) const override {
			return mBluePrint->mapToMatrixArrayIndex(keyFrameSID);
		};

	private:
		const VobBluePrint* mBluePrint;
	};

	return std::make_unique<Generator>(this);
}

void nex::VobBluePrint::addKeyFrameAnimations(std::vector<std::unique_ptr<KeyFrameAnimation>> vec)
{
	for (auto& ani : vec) {

		const auto sid = SID(ani->getName());

		// Note: overwrites exisiting keyframe animation
		mKeyFrameAnis[sid] = std::move(ani);
	}

	// resort
	createSortedAnis();
}

const std::unordered_map<nex::Sid, std::unique_ptr<nex::KeyFrameAnimation>>& nex::VobBluePrint::getKeyFrameAnimations() const
{
	return mKeyFrameAnis;
}

const std::vector<nex::KeyFrameAnimation*>& nex::VobBluePrint::getKeyFrameAnimationsSorted() const
{
	return mKeyFrameAnisSorted;
}

unsigned nex::VobBluePrint::getMaxChannelCount() const
{
	return static_cast<unsigned>(mBluePrintChildVobNameSIDToMatrixIndex.size());
}

nex::Sid nex::VobBluePrint::getBluePrintRootNameSID() const
{
	return mBluePrintRootNameSID;
}

unsigned nex::VobBluePrint::mapToMatrixArrayIndex(const nex::Vob& vob) const
{
	//if (vob.getBluePrint() != this) {
	//	throw_with_trace(std::invalid_argument("Vob isn't connected to the blue-print!" + vob.getName()));
	//}

	const auto sid = vob.getBluePrintNodeNameSID();
	return mapToMatrixArrayIndex(sid);
}

unsigned nex::VobBluePrint::mapToMatrixArrayIndex(nex::Sid sid) const
{

	auto it = mBluePrintChildVobNameSIDToMatrixIndex.find(sid);

	if (it == end(mBluePrintChildVobNameSIDToMatrixIndex)) {
		throw_with_trace(std::invalid_argument("SID isn't mapped to an array index: " + std::to_string(sid)));
	}

	return it->second;
}

const std::unordered_map<nex::Sid, unsigned>& nex::VobBluePrint::getMapping() const
{
	return mBluePrintChildVobNameSIDToMatrixIndex;
}

int nex::VobBluePrint::fillMap(const nex::Vob& vob, int currentIndex)
{
	const auto sid = SID(vob.getName());

	if (mBluePrintChildVobNameSIDToMatrixIndex.find(sid) != end(mBluePrintChildVobNameSIDToMatrixIndex)) {
		bool t = false;
	}

	mBluePrintChildVobNameSIDToMatrixIndex[sid] = currentIndex;

	for (const auto& child : vob.getChildren()) {
		currentIndex = fillMap(*child.get(), currentIndex + 1);
	}

	// return the current index
	return currentIndex;
}

void nex::VobBluePrint::createSortedAnis()
{
	mKeyFrameAnisSorted.clear();
	mKeyFrameAnisSorted.reserve(mKeyFrameAnis.size());

	for (auto& pair : mKeyFrameAnis) {
		mKeyFrameAnisSorted.push_back(pair.second.get());
	}

	std::sort(begin(mKeyFrameAnisSorted), end(mKeyFrameAnisSorted), [](const auto* a, const auto* b) {
		return std::lexicographical_compare(begin(a->getName()), end(a->getName()),
			begin(b->getName()), end(b->getName()));
		});
}