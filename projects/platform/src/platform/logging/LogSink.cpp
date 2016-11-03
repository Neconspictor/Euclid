#include <platform/logging/LogSink.hpp>
#include <platform/util/Util.hpp>
#include <iostream>
#include <fstream>

using namespace std;

namespace platform
{
	LogSink::LogSink(const LogSink& sink) :
		mWrapper(sink.mWrapper->clone())
	{
	}

	LogSink& LogSink::operator=(LogSink sink)
	{
		// the function signature already specifies a copy, so we only need to move the result from the temporary
		mWrapper = move(sink.mWrapper);
		return *this;
	}

	bool LogSink::operator == (const LogSink& sink) const {
		return (mWrapper.get() == sink.mWrapper.get());
	}

	void LogSink::forward(
		const string& prefix,
		const LogMessage::Meta& meta,
		const string& message
		) const {
		mWrapper->forward(prefix, meta, message);
	}

	LogSink makeConsoleSink() {
		return LogSink([](
			const string& prefix,
			const LogMessage::Meta& meta,
			const string& message
			) {
			cout
				<< prefix
				<< " "
				<< meta.level
				<< " : "
				<< message
				<< "\n";
		});
	}

	struct FileSink {
		FileSink(const string& filename) :
			mFile(make_shared<ofstream>(filename))
		{
			if (!mFile->good()) {
				string message = "Failed to open file sink: ";
				message.append(filename);
				throw runtime_error(message);
			}
		}

		void operator()(const string& prefix, const LogMessage::Meta& meta, const string& message) {
			string file = meta.mFile;
			file = util::makeAbsolute(file);
			(*mFile)
				<< prefix
				<< " "
				<< meta.level
				<< " : "
				<< message
				<< " ("
				<< util::relativePathToBuildDirectory(file)
				<< ":"
				<< meta.mLine
				<< endl;
			mFile->flush();
		}

		shared_ptr<ofstream> mFile; // by using shared_ptr this can be copied around
	};


	LogSink makeFileSink(const string& filename) {
		return LogSink(FileSink(filename)); // implicitly converted to LogSink
	}
}