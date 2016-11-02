#include <platform/FileSystem.hpp>
#include <iostream>
#include <sstream>
#include <fstream>

using namespace std;

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
		if (shaderStreamFile.fail())
		{
			cerr << "Error: filesystem::loadFileIntoString(): Couldn't opened file: "
				<< filePath << endl;
		}

		if (shaderStreamFile.bad())
		{
			cerr << "Error: filesystem::loadFileIntoString : Couldn't read file properly." << endl;
		}
		loadingWasSuccessful = false;
		*destination = "";
	}

	//clear exceptions as close shouldn't throw any exceptions!
	shaderStreamFile.exceptions(0);
	shaderStreamFile.close();

	return loadingWasSuccessful;
}