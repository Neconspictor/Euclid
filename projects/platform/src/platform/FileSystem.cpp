#include <platform/FileSystem.hpp>
#include <iostream>
#include <sstream>
#include <fstream>
#include <platform/logging/LoggingClient.hpp>
#include <platform/logging/GlobalLoggingServer.hpp>

using namespace std;
using namespace platform;

bool filesystem::loadFileIntoString(const string& filePath, string* destination)
{
	ifstream shaderStreamFile;
	bool loadingWasSuccessful = true;

	// ensure ifstream can throw exceptions!
	shaderStreamFile.exceptions(ifstream::failbit | ifstream::badbit);

	try
	{
		shaderStreamFile.open(filePath);
		stringstream shaderStream;

		// read file content to stream
		shaderStream << shaderStreamFile.rdbuf();
		*destination = shaderStream.str();
	}
	catch (ifstream::failure e)
	{
		LoggingClient logClient(getLogServer());
		logClient.setPrefix("[filesystem::loadFileIntoString()]");

		if (shaderStreamFile.fail())
		{
			LOG(logClient, Error) << "Couldn't opened file: "
				<< filePath;
		}

		if (shaderStreamFile.bad())
		{
			LOG(logClient, Error) << "Couldn't read file properly.";
		}
		loadingWasSuccessful = false;
		*destination = "";
	}

	//clear exceptions as close shouldn't throw any exceptions!
	shaderStreamFile.exceptions(0);
	shaderStreamFile.close();

	return loadingWasSuccessful;
}