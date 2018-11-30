#include "CommentStateTracker.hpp"
#include <stdexcept>
#include "nex/util/ExceptionHandling.hpp"

using namespace nex;

CommentStateTracker::CommentStateTracker(CommentState initialState) : mState(initialState)
{
}

void CommentStateTracker::update(const StreamPos& streamPos)
{
	char current;
	char previous;
	char previous2;
	char next;


	if (streamPos.pos - 1 >= streamPos.start)
		previous = *(streamPos.pos - 1);
	else
		previous = *streamPos.start;

	if (streamPos.pos - 2 >= streamPos.start)
		previous2 = *(streamPos.pos - 2);
	else
		previous2 = *streamPos.start;

	current = *streamPos.pos;

	if (streamPos.pos + 1 <= streamPos.end)
		next = *(streamPos.pos + 1);
	else
		next = *streamPos.end;

	bool isSlash = current == '/';

	bool previousIsSlash = previous == '/';
	bool previousIsNewLine = previous == '\n';

	bool previous2IsAsterix = previous2 == '*';

	bool nextIsSlash = next == '/';
	bool nextIsAsterix = next == '*';

	switch (mState)
	{
	case CommentState::NO_COMMENT:
		if (isSlash && nextIsSlash)
			mState = CommentState::LINE_COMMENT;

		if (isSlash && nextIsAsterix)
			mState = CommentState::MULTI_LINE_COMMENT;

		break;

	case CommentState::LINE_COMMENT:
		if (previousIsNewLine)
			mState = CommentState::NO_COMMENT;
		break;

	case CommentState::MULTI_LINE_COMMENT:
		if (previous2IsAsterix && previousIsSlash)
			mState = CommentState::NO_COMMENT;
		break;

	default: throw_with_trace(std::runtime_error("Not valid CommentState: " + to_underlying(mState)));
	}
}

bool CommentStateTracker::isActive() const
{
	return isCommentState(mState);
}

bool CommentStateTracker::isCommentState(CommentState state)
{
	return state == CommentState::LINE_COMMENT
		|| state == CommentState::MULTI_LINE_COMMENT;
}