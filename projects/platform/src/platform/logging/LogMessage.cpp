#include <platform/logging/LogMessage.hpp>
#include <platform/logging/Logger.hpp>

using namespace std;

namespace platform
{
	LogMessage& LogMessage::operator << (ostream& (*fn)(ostream& os)) {
		fn(mBuffer);
		return *this;
	}

	LogMessage::LogMessage(LogMessage& other)
	{
		this->mBuffer << other.mBuffer.str();
		this->mOwner = other.mOwner;
		this->mMeta.mFile = other.mMeta.mFile;
		this->mMeta.mLine = other.mMeta.mLine;
		this->mMeta.level = other.mMeta.level;
	}

	/*LogMessage::LogMessage(const LogMessage& other)
	{
	this->mBuffer << other.mBuffer.str();
	this->mOwner = other.mOwner;
	cout << "LogMessage::LogMessage(const LogMessage& other) called!" << endl;
	}*/

	LogMessage::LogMessage(LogMessage&& other) :
		mBuffer(move(other.mBuffer)),
		mOwner(move(other.mOwner)),
		mMeta(move(other.mMeta))
	{
		other.mOwner = nullptr;
	}

	LogMessage::LogMessage(LogLevel level, const string& file, const string& function, int line, Logger* owner) :
		mOwner(owner)
	{
		mMeta.mFile = file;
		mMeta.mLine = line;
		mMeta.mFunction = function;
		mMeta.level = level;
	}

	LogMessage::LogMessage()
	{
		mOwner = nullptr;
	}

	LogMessage::~LogMessage() {
		if (mOwner)
		{
			mOwner->flush(*this);
		}
	}
}
