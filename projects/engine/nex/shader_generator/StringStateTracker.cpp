#include "StringStateTracker.hpp"

StringStateTracker::StringStateTracker(bool isStringActive) : mIsStringActive(isStringActive)
{
}

void StringStateTracker::update(const StreamPos& streamPos)
{
	bool isQuotationMark = *streamPos.pos == '"';

	if (isQuotationMark)
		mIsStringActive = !mIsStringActive;
}

bool StringStateTracker::isActive() const
{
	return mIsStringActive;
}