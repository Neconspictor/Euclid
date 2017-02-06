#include <texture/opengl/ImageLoaderGL.hpp>
#include <platform/FileSystem.hpp>
#include <platform/util/Util.hpp>
#include <platform/logging/GlobalLoggingServer.hpp>

using namespace std;
using namespace platform;

ImageLoaderGL::ImageLoaderGL() : logClient(getLogServer())
{}

GenericImageGL ImageLoaderGL::loadImageFromDisc(string fileName)
{
	GenericImageGL imageData;
	memset(&imageData, 0, sizeof(imageData));
	char* bytes = filesystem::getBytesFromFile(fileName);
	if (!bytes)
	{
		throw runtime_error("ImageLoaderGL::loadImageFromDisc(string): Couldn't load image: " + fileName);
	}

	// get file extension
	vector<string> tokens = util::tokenize(fileName, "\\."); // '.' has to be escaped in regex patterns!
	if (tokens.size() <= 1)
		throw runtime_error("ImageLoaderGL::loadImageFromDisc(string): Couldn't detect a valid file extension: " + fileName);

	string extension = tokens.back();

	// make extension upper case for easier string comparisons
	transform(extension.begin(), extension.end(), extension.begin(), ::toupper);

	if (extension.compare("DDS") == 0)
	{
		LOG(logClient, Debug) << "image file is a dds file: " << fileName;
		imageData.pixels = bytes;
		imageData.bufSize = sizeof(bytes);
	} else
	{
		LOG(logClient, Error) << "image type couldn't be detected from: " << fileName;
	}

	return imageData;
}