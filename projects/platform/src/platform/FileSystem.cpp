#include <platform/FileSystem.hpp>
#include <sstream>
#include <fstream>
#include <platform/logging/LoggingClient.hpp>
#include <platform/logging/GlobalLoggingServer.hpp>
#include <boost/filesystem.hpp>

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
		logClient.setPrefix("[FileSystem::loadFileIntoString()]");

		if (shaderStreamFile.fail())
		{
			LOG(logClient, Error) << "Couldn't open file: "
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

char* filesystem::getBytesFromFile(const string& filePath)
{
	ifstream file;
	MemoryWrapper content(nullptr);
	streampos fileSize = 0;
	LoggingClient logClient(getLogServer());
	logClient.setPrefix("[FileSystem::getBytesFromFile()]");

	// ensure ifstream can throw exceptions!
	file.exceptions(ifstream::failbit | ifstream::badbit);

	try
	{
		file.open(filePath, ios::binary);
		filebuf* buffer = file.rdbuf();
		
		//get file size
		fileSize  = buffer->pubseekoff(0, file.end, file.in);
		buffer->pubseekpos(0, file.in);

		// allocate memory to contain file data
		char* memory = new char[fileSize];
		content.setContent(memory);

		//copy data to content buffer
		buffer->sgetn(*content, fileSize);
	}
	catch (ifstream::failure e)
	{
		if (file.fail())
		{
			LOG(logClient, Error) << "Couldn't open file: " << filePath;
		}

		if (file.bad())
		{
			LOG(logClient, Error) << "Couldn't read file properly.";
		}
	} 
	catch (bad_alloc e)
	{
		LOG(logClient, Error) << "Couldn't allocate memory of size: " << to_string(fileSize);
	}

	//clear exceptions as close shouldn't throw any exceptions!
	file.exceptions(0);
	file.close();

	char* result = *content;
	content.setContent(nullptr);

	return result;
}

streampos filesystem::getFileSize(const string& filePath)
{
	ifstream file;
	streampos size = 0;

	// ensure ifstream can throw exceptions!
	file.exceptions(ifstream::failbit | ifstream::badbit);

	try
	{
		file.open(filePath, ios::binary);
		filebuf* buffer = file.rdbuf();

		//get file size
		size = buffer->pubseekoff(0, file.end, file.in);
		buffer->pubseekpos(0, file.in);
	}
	catch (ifstream::failure e)
	{
		LoggingClient logClient(getLogServer());
		logClient.setPrefix("[FileSystem::loadFileIntoString()]");

		if (file.fail())
		{
			LOG(logClient, Error) << "Couldn't open file: "
				<< filePath;
		}

		if (file.bad())
		{
			LOG(logClient, Error) << "Couldn't read file properly.";
		}
	}

	//clear exceptions as close shouldn't throw any exceptions!
	file.exceptions(0);
	file.close();

	return size;
}

vector<string> filesystem::getFilesFromFolder(const string& folderPath, bool skipSubFolders)
{
	using namespace boost::filesystem;
	path folder(folderPath);
	
	if (!is_directory(folder) || !exists(folder))
	{
		return vector<string>();
	}

	vector<string> result;

	vector<string> subPaths;

	for (directory_iterator it(folder); it != directory_iterator(); ++it)
	{
		path path = it->path();
		if (!skipSubFolders && is_directory(path))
		{
			subPaths.push_back(path.generic_string());
		} else
		{
			result.push_back(path.generic_string());
		}
	}

	for (auto& path : subPaths)
	{
		vector<string> subResult = getFilesFromFolder(path, false);
		result.insert(result.begin(), subResult.begin(), subResult.end());
	}

	return result;
}