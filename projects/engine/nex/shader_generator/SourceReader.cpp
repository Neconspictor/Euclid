#include "SourceFileConsumer.hpp"
#include "SourceReader.hpp"
#include <fstream>
#include "CommentStateTracker.hpp"
#include "StringStateTracker.hpp"
#include <sstream>
#include <cassert>

void Parser::addConsumer(SourceFileConsumer* consumer)
{
	mConsumers.push_back(consumer);
}

void Parser::parse(StreamPos& streamPos)
{
	CommentStateTracker commentTracker;
	StringStateTracker stringTracker;

	ExclusiveTrackCollection trackCollection;
	trackCollection.addTracker(&commentTracker);
	trackCollection.addTracker(&stringTracker);

	StreamPositionTracker positionTracker;

	for (; streamPos.pos != streamPos.end; ++streamPos.pos)
	{
		trackCollection.update(streamPos);

		SourceFileConsumer::StreamDesc desc;
		desc.position = &streamPos;
		desc.isComment = commentTracker.isActive();

		positionTracker.consume(desc);

		for (auto& consumer : mConsumers)
		{
			try
			{
				consumer->consume(desc);
			}
			catch (const ParseException& e)
			{
				int line = positionTracker.getLine();
				int cursor = positionTracker.getColumn();

				std::stringstream ss;
				ss << "ERROR: Parsing error at (" << line << ":" << cursor << "): "
					<< e.what();
				throw ParseException(ss.str().c_str());
			}
		}
	}
}

SourceFileParser::SourceFileParser(const std::filesystem::path& filePath) : Parser(), mFilePath(filePath)
{
}

void SourceFileParser::read(std::vector<char>* result)
{
	{
		std::ifstream file;
		file.exceptions(std::ifstream::failbit | std::ifstream::badbit);
		file.open(mFilePath);
		std::copy(
			std::istreambuf_iterator<char>(file.rdbuf()),
			std::istreambuf_iterator<char>(),
			std::back_inserter(*result));
	}

	StreamPos streamPos{ &result->front(), &result->front(), &result->back() + 1 };
	parse(streamPos);
}

SourceMemoryParser::SourceMemoryParser(const std::vector<char>* source) : Parser(), mSource(source)
{
	assert(mSource != nullptr);
}

void SourceMemoryParser::read()
{
	StreamPos streamPos{ &mSource->front(), &mSource->front(), &mSource->back() + 1 };
	parse(streamPos);
}