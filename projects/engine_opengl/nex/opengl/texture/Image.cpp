#include <nex/opengl/texture/Image.hpp>
#include <nex/FileSystem.hpp>
//#include <DDS.h>
#include <boost/interprocess/streams/bufferstream.hpp>
#include <nex/util/ExceptionHandling.hpp>
#include <nex/util/StringUtils.hpp>


using namespace boost::interprocess;

using namespace std;
using namespace nex;

GenericImageGL::GenericImageGL(GenericImageGL&& o) noexcept :
pixels(std::move(o.pixels)), width(o.width), height(o.height), components(o.components),
bufSize(o.bufSize), pixelSize(o.pixelSize), format(o.format)
{
}

GenericImageGL& GenericImageGL::operator=(GenericImageGL&& o) noexcept
{
	if (this == &o) return *this;

	pixels = std::move(o.pixels);
	width = (o.width); 
	height = (o.height); 
	components = (o.components);
	bufSize = (o.bufSize); 
	pixelSize = (o.pixelSize); 
	format = (o.format);

	return *this;
}

void GenericImageGL::load(GenericImageGL* dest, FILE* file)
{
	std::fread(dest, sizeof(GenericImageGL), 1, file);
	int err;
	if ((err = std::ferror(file)) != 0)
		throw_with_trace(std::runtime_error("Couldn't read from file: Error code =  " + std::to_string(err)));

	dest->pixels = new char[dest->bufSize];

	std::fread(&*dest->pixels, dest->bufSize, 1, file);

	if ((err = std::ferror(file)) != 0)
		throw_with_trace(std::runtime_error("Couldn't read from file: Error code =  " + std::to_string(err)));
}

void GenericImageGL::write(const GenericImageGL& image, FILE* file)
{
	int err;
	std::fwrite(&image, sizeof(GenericImageGL), 1, file);
	if ((err = std::ferror(file)) != 0)
		throw_with_trace(std::runtime_error("Couldn't write to file: Error code =  " + std::to_string(err)));

	std::fwrite(&*image.pixels, image.bufSize, 1, file);
	if ((err = std::ferror(file)) != 0)
		throw_with_trace(std::runtime_error("Couldn't write to file: Error code =  " + std::to_string(err)));
}

StoreImageGL::StoreImageGL(StoreImageGL&& o) noexcept :
images(std::move(o.images)), mipmapCounts(std::move(o.mipmapCounts)), sideCount(o.sideCount)
{
	o.sideCount = 0;
}

StoreImageGL& StoreImageGL::operator=(StoreImageGL&& o) noexcept
{
	if (this == &o) return *this;

	images = std::move(o.images);
	mipmapCounts = std::move(o.mipmapCounts);
	sideCount = o.sideCount;

	o.sideCount = 0;

	return *this;
}


void StoreImageGL::load(StoreImageGL* dest, const char* filePath)
{
	FILE* file = nullptr;
	errno_t err;
	if ((err = fopen_s(&file, filePath, "rb")) != 0)
		throw_with_trace(std::runtime_error("Couldn't read from file " + std::string(filePath)));

	std::fread(dest, sizeof(StoreImageGL), 1, file);

	if (std::ferror(file) != 0)
		throw_with_trace(std::runtime_error("Couldn't read from file " + std::string(filePath)));

	dest->mipmapCounts = new unsigned short[dest->sideCount];
	std::fread(dest->mipmapCounts.get(), sizeof(unsigned short) * dest->sideCount, 1, file);

	if (std::ferror(file) != 0)
		throw_with_trace(std::runtime_error("Couldn't read from file " + std::string(filePath)));
	
	dest->images = new GuardArray<GenericImageGL>[dest->sideCount];

	
	for (unsigned i = 0; i < dest->sideCount; ++i)
	{
		dest->images[i] = new GenericImageGL[dest->mipmapCounts[i]];

		for (unsigned j = 0; j < dest->mipmapCounts[i]; ++j)
		{
			GenericImageGL::load(&dest->images[i][j], file);
		}
	}


	fclose(file);
}

void StoreImageGL::write(const StoreImageGL& source, const char* filePath)
{
	FILE* file = nullptr;
	errno_t err;
	if ((err = fopen_s(&file, filePath, "w+b")) != 0)
		throw_with_trace(std::runtime_error("Couldn't write to file " + std::string(filePath)));

	std::fwrite(&source, sizeof(StoreImageGL), 1, file);
	std::fwrite(source.mipmapCounts.get(), sizeof(unsigned short), source.sideCount, file);

	for (unsigned i = 0; i < source.sideCount; ++i)
	{
		auto& array = source.images[i];
		for (unsigned j = 0; j < source.mipmapCounts[i]; ++j)
		{
			GenericImageGL::write(array[j], file);
		}
	}

	if (std::ferror(file) != 0)
		throw_with_trace(std::runtime_error("Couldn't write to file " + std::string(filePath)));

	fclose(file);
}

ImageLoaderGL::ImageLoaderGL() : m_logger("ImageLoaderGL")
{}

GenericImageGL ImageLoaderGL::loadImageFromDisc(string fileName)
{
	GenericImageGL imageData;
	memset(&imageData, 0, sizeof(imageData));
	streampos fileSize;
	char* bytes = FileSystem::getBytesFromFile(fileName, &fileSize);
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
		LOG(m_logger, Debug) << "image file is a dds file: " << fileName;
		LOG(m_logger, Debug) << "file size (bytes): " << fileSize;
		imageData = loadDDSFile(bytes, fileSize);
	} else
	{
		LOG(m_logger, Error) << "image type couldn't be detected from: " << fileName;
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