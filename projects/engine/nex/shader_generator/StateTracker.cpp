#include "StateTracker.hpp"

using namespace nex;

void ExclusiveTrackCollection::addTracker(StateTracker* tracker)
{
	mTrackers.push_back(tracker);
}

bool ExclusiveTrackCollection::isActive() const
{
	return mActiveTracker != nullptr;
}

void ExclusiveTrackCollection::update(const StreamPos& streamPos)
{
	if (mActiveTracker)
	{
		mActiveTracker->update(streamPos);
		if (!mActiveTracker->isActive()) mActiveTracker = nullptr;
		return;
	}

	for (auto& tracker : mTrackers)
	{
		tracker->update(streamPos);
		if (tracker->isActive())
		{
			mActiveTracker = tracker;

			// Remember: If any tracker is active, we only update the active one!
			return;
		}
	}
}