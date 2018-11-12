#pragma once
#include "SourceFileConsumer.hpp"
#include <list>

class StateTracker
{
public:
	virtual ~StateTracker() = default;

	virtual bool isActive() const = 0;
	virtual void update(const StreamPos& streamPos) = 0;
};

class ExclusiveTrackCollection : public StateTracker
{
public:

	virtual ~ExclusiveTrackCollection() = default;

	void addTracker(StateTracker* tracker);

	/**
	 * Checks whether any of the registered StateTrackers is active.
	 */
	bool isActive() const override;

	void update(const StreamPos& streamPos) override;

private:
	std::list<StateTracker*> mTrackers;
	StateTracker* mActiveTracker = nullptr;
};