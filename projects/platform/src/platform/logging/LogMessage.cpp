#include <platform/logging/LogMessage.hpp>
#include <platform/logging/LoggingClient.hpp>

using namespace std;

namespace platform
{
	LogMessage& LogMessage::operator << (ostream& (*fn)(ostream& os)) {
		fn(buffer);
		return *this;
	}

	LogMessage::LogMessage(LogMessage& other)
	{
		this->buffer << other.buffer.str();
		this->client = other.client;
		this->meta.mFile = other.meta.mFile;
		this->meta.mLine = other.meta.mLine;
		this->meta.level = other.meta.level;
	}

	/*LogMessage::LogMessage(const LogMessage& other)
	{
	this->mBuffer << other.mBuffer.str();
	this->mOwner = other.mOwner;
	cout << "LogMessage::LogMessage(const LogMessage& other) called!" << endl;
	}*/

	LogMessage::LogMessage(LogMessage&& other) :
		buffer(move(other.buffer)),
		client(move(other.client)),
		meta(move(other.meta))
	{
		other.client = nullptr;
	}

	LogMessage::LogMessage(LogLevel level, const string& file, const string& function, int line, const LoggingClient* owner) :
		client(owner)
	{
		meta.mFile = file;
		meta.mLine = line;
		meta.mFunction = function;
		meta.level = level;
	}

	LogMessage::LogMessage()
	{
		client = nullptr;
	}

	LogMessage::~LogMessage() {
		if (client)
		{
			client->flush(*this);
		}
	}
}