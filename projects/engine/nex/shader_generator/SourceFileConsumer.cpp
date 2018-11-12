#include "SourceFileConsumer.hpp"
#include <sstream>

void LineCounter::consume(const StreamDesc& desc)
{
	char c = *desc.position->pos;

	if (c == '\n')
		++mLineCount;
}

int LineCounter::getLineCount() const
{
	return mLineCount;
}

void StreamPositionTracker::consume(const StreamDesc& desc)
{
	++mColumn;

	const char* pos = desc.position->pos;
	const char* start = desc.position->start;
	char previous;

	if (pos - 1 >= start)
		previous = *(pos - 1);
	else
		previous = *start;

	if (previous == '\n')
	{
		++mLine;
		mColumn = 1;
	}
}

int StreamPositionTracker::getLine() const
{
	return mLine;
}

int StreamPositionTracker::getColumn() const
{
	return mColumn;
}

void OffsetIncludeCalculator::consume(const StreamDesc& desc)
{
	char c = *desc.position->pos;

	if (c == '\n')
	{
		++mLineCount;

		if (mLine.find("#include") != std::string::npos)
			mOffset = mLineCount;

		mLine.clear();

		return;
	}

	if (desc.isComment) return;

	mLine.push_back(c);
}

int OffsetIncludeCalculator::getOffset() const
{
	return mOffset;
}

void IncludeCollector::consume(const StreamDesc& desc)
{
	const char* pos = desc.position->pos;
	const char* start = desc.position->start;
	char c = *pos;
	char previous;

	if (pos - 1 >= start)
		previous = *(pos - 1);
	else
		previous = *start;

	bool isQuotationMarks = c == '\"';
	bool isNewLine = c == '\n';

	bool previousIsIgnorable = isIgnorable(previous);

	if (!desc.isComment && !isIgnorable(c) &&
		!(isQuotationMarks && (mState == State::PATH)))
		mBuffer.push_back(c);

	if (desc.isComment && !isNewLine) return;

	if (isNewLine)
	{
		mBuffer.clear();
	}

	switch (mState)
	{
	case State::DEFAULT:
		if (mBuffer == "#include")
		{
			mState = State::INCLUDE_DIRECTIVE;
		}
		break;
	case State::INCLUDE_DIRECTIVE:
		if (isQuotationMarks && previousIsIgnorable)
		{
			mState = State::PATH;
			mBuffer.clear();
		}
		else if (!isIgnorable(c))
			throw ParseException("Expected quotation mark after include directive.");

		break;
	case State::PATH:
		if (isNewLine) {
			throw ParseException("Unexpected end of line.");
		}

		if (isQuotationMarks)
		{
			Include include;
			include.filePath = std::move(mBuffer);
			include.lineBegin = getLineBegin(desc) - start;
			include.lineEnd = getLineEnd(desc) - start;
			mIncludes.emplace_back(std::move(include));

			mBuffer.clear();
			mState = State::POST_INCLUDE;
		}

		break;

	case State::POST_INCLUDE:

		if (isNewLine)
		{
			mState = State::DEFAULT;
			mBuffer.clear();

		}
		else if (!isIgnorable(c))
		{
			throw ParseException("Unexpected token after include directive.");
		}

		break;
	default:;
	}
}

const std::vector<IncludeCollector::Include>& IncludeCollector::getIncludes() const
{
	return mIncludes;
}

bool IncludeCollector::isIgnorable(char c)
{
	return c == ' '
		|| c == '\t';
}

const char* IncludeCollector::getLineBegin(const StreamDesc& desc)
{
	const char* pos = desc.position->pos;
	const char* start = desc.position->start;

	auto it = pos - 1;
	for (; it >= start && (*it != '\n'); --it);

	//if (it < start) it = start;

	return it + 1;
}

const char* IncludeCollector::getLineEnd(const StreamDesc& desc)
{
	const char* pos = desc.position->pos;
	const char* end = desc.position->end;

	auto it = pos + 1;
	for (; it <= end && (*it != '\n'); ++it);

	if (it > end) it = end;

	return it;
}

std::ostream& operator<<(std::ostream& os, const IncludeCollector::Include& include)
{
	os << include.filePath << ", " << include.lineBegin << ", " << include.lineEnd;
	return os;
}