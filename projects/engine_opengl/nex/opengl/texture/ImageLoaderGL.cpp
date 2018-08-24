#include <nex/opengl/texture/ImageLoaderGL.hpp>
#include <nex/FileSystem.hpp>
#include <nex/util/Util.hpp>
#include <nex/logging/GlobalLoggingServer.hpp>
//#include <DDS.h>
#include <boost/interprocess/streams/bufferstream.hpp>


using namespace boost::interprocess;

using namespace std;
using namespace nex;

ImageLoaderGL::ImageLoaderGL() : logClient(getLogServer())
{}

GenericImageGL ImageLoaderGL::loadImageFromDisc(string fileName)
{
	GenericImageGL imageData;
	memset(&imageData, 0, sizeof(imageData));
	streampos fileSize;
	char* bytes = nex::filesystem::getBytesFromFile(fileName, &fileSize);
	if (!bytes)
	{
		throw_with_trace(runtime_error("ImageLoaderGL::loadImageFromDisc(string): Couldn't load image: " + fileName));
	}

	// get file extension
	vector<string> tokens = util::tokenize(fileName, "\\."); // '.' has to be escaped in regex patterns!
	if (tokens.size() <= 1)
		throw_with_trace(runtime_error("ImageLoaderGL::loadImageFromDisc(string): Couldn't detect a valid file extension: " + fileName));

	string extension = tokens.back();

	// make extension upper case for easier string comparisons
	transform(extension.begin(), extension.end(), extension.begin(), ::toupper);

	if (extension.compare("DDS") == 0)
	{
		LOG(logClient, Debug) << "image file is a dds file: " << fileName;
		LOG(logClient, Debug) << "file size (bytes): " << fileSize;
		imageData = loadDDSFile(bytes, fileSize);
	} else
	{
		LOG(logClient, Error) << "image type couldn't be detected from: " << fileName;
	}

	delete[] bytes;

	return imageData;
}

GenericImageGL ImageLoaderGL::loadDDSFile(char* imageData, streampos fileSize)
{
	return GenericImageGL();
	/*DirectX::DDS_HEADER ddsd;
	char filecode[4];
	GenericImageGL content;
	memset(&content, 0, sizeof(GenericImageGL));

	bufferstream istream(imageData, fileSize);
	istream.read(filecode, 4);

	if (strncmp(filecode, "DDS ", 4) != 0)
	{
		LOG(logClient, Error) << "ImageLoaderGL::loadDDSFile: image is no dds file!";
		return content;
	}

	istream.read((char*)&ddsd, sizeof(DirectX::DDS_HEADER));
	size_t sizeDDSd = sizeof(DirectX::DDS_HEADER);
	content.bufSize = ddsd.dwMipMapCount > 1 ? ddsd.dwPitchOrLinearSize * 2 : ddsd.dwPitchOrLinearSize;
	content.pixels = new char[content.bufSize];
	istream.read((char*)content.pixels, content.bufSize);

	content.width = ddsd.dwWidth;
	content.height = ddsd.dwHeight;
	const uint32_t DXT1 =  MAKEFOURCC('D', 'X', 'T', '1');
	const uint32_t DXT3 = MAKEFOURCC('D', 'X', 'T', '3');
	const uint32_t DXT5 = MAKEFOURCC('D', 'X', 'T', '5');

	content.components = (ddsd.ddspf.dwFourCC == DXT1) ? 3 : 4;
	content.numMipmaps = ddsd.dwMipMapCount;

	/*

	TODO !!!


	switch(ddsd.ddspf.dwFourCC)
	{
	case DXT1:
		content.format = GL_COMPRESSED_RGBA_S3TC_DXT1_EXT;
		break;
	case DXT3:
		content.format = GL_COMPRESSED_RGBA_S3TC_DXT3_EXT;
		break;
	case DXT5:
		content.format = GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
		break;
	default:
		delete[] content.pixels;
		memset(&content, 0, sizeof(content));
	}*/
	//return content;
}