#pragma once
#include "StateTracker.hpp"

class StringStateTracker : public StateTracker
{
public:
	StringStateTracker(bool isStringActive = false);

	void update(const StreamPos& streamPos) override;
	bool isActive() const override;

private:
	bool mIsStringActive;
};
