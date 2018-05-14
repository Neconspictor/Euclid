#include <platform/logging/LogEndpoint.hpp>
#include <iostream>
#include <fstream>

using namespace std;

namespace platform
{
	LogEndpoint::LogEndpoint(const LogEndpoint& sink) :
		mWrapper(sink.mWrapper->clone())
	{
	}

	LogEndpoint& LogEndpoint::operator=(LogEndpoint sink)
	{
		// the function signature already specifies a copy, so we only need to move the result from the temporary
		mWrapper = move(sink.mWrapper);
		return *this;
	}

	bool LogEndpoint::operator == (const LogEndpoint& sink) const {
		return (mWrapper.get() == sink.mWrapper.get());
	}

	void LogEndpoint::log(
		const string& prefix,
		const LogMessage::Meta& meta,
		const string& message
		) const {
		mWrapper->forward(prefix, meta, message);
	}

	LogEndpoint makeConsoleEndpoint() {
		return LogEndpoint([](
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
		FileSink(const string& filename)
			//: mFile(make_shared<ofstream>(filename))
		{
			ofstream file(filename);
			if (!file.good()) {
				string message = "Failed to open file sink: ";
				message.append(filename);
				throw runtime_error(message);
			}

			this->filename = filename;
		}

		void operator()(const string& prefix, const LogMessage::Meta& meta, const string& message) {
			//int buildPathSize = sizeof(NEC_BUILD_PATH) -1;
			//auto file = meta.mFile.substr(buildPathSize, meta.mFile.size());
			ofstream logFile(filename, ios::app);
			if (!logFile.good()) {
				string message = "Failed to open file sink: ";
				message.append(filename);
				throw runtime_error(message);
			}
			logFile
				<< prefix
				<< " "
				<< meta.level
				<< " : "
				<< message
				//<< " ("
				//<< util::relativePathToBuildDirectory(file)
				//<< file
				//<< ":"
				//<< meta.mLine
				<< endl;
		}

		//shared_ptr<ofstream> mFile; // by using shared_ptr this can be copied around
		string filename;
	};


	LogEndpoint makeFileEndpoint(const string& filename) {
		return LogEndpoint(FileSink(filename)); // implicitly converted to LogSink
	}
}