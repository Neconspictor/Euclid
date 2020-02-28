#include <nex/scene/VobBluePrint.hpp>

nex::VobBluePrint::VobBluePrint(std::unique_ptr<Vob> bluePrint) : mVobHierarchy(std::move(bluePrint))
{
}

std::unique_ptr<nex::Vob> nex::VobBluePrint::createBluePrint() const
{
	auto copy = mVobHierarchy->createBluePrintCopy();

	copy->setBluePrint(this);

	return copy;
}

void nex::VobBluePrint::addKeyFrameAnimations(std::unordered_map<nex::Sid, std::unique_ptr<KeyFrameAnimation>> aniMap)
{
	for (auto& pair : aniMap) {
		mKeyFrameAnis[pair.first] = std::move(pair.second);
	}
}

const std::unordered_map<nex::Sid, std::unique_ptr<nex::KeyFrameAnimation>>& nex::VobBluePrint::getKeyFrameAnimations() const
{
	return mKeyFrameAnis;
}