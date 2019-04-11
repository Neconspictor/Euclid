#include <nex/texture/Image.hpp>
#include <nex/FileSystem.hpp>
//#include <DDS.h>
#include <boost/interprocess/streams/bufferstream.hpp>
#include <nex/util/ExceptionHandling.hpp>
#include <nex/util/StringUtils.hpp>


using namespace boost::interprocess;

using namespace std;
using namespace nex;

struct SerializedGenericImage
{
	size_t bufferSize = 0;
	unsigned width = 0;
	unsigned height = 0;
	size_t pixelSize = 0;
	unsigned components = 0;
	unsigned format = 0;

	SerializedGenericImage() = default;

	SerializedGenericImage(const GenericImage& image)
	{
		bufferSize = image.pixels.size();
		width = image.width;
		height = image.height;
		pixelSize = image.pixelSize;
		components = image.components;
		format = image.format;
	}

	void fill(GenericImage& image) const
	{
		image.width = width;
		image.height = height;
		image.pixelSize = pixelSize;
		image.pixels.resize(bufferSize);
		image.components = components;
		image.format = format;
	}
};


struct SerializedStoreImage
{
	unsigned short levelCount = 1;
	unsigned short mipMapCount = 1;
	bool isCubeMap = false;
	unsigned textureTarget = 0;

	SerializedStoreImage() = default;

	SerializedStoreImage(const StoreImage& image)
	{
		levelCount = image.images.size();
		mipMapCount = image.mipmapCount;
		textureTarget = static_cast<unsigned>(image.textureTarget);
	}

	void fill(StoreImage& image) const
	{
		image.images.resize(levelCount);
		image.mipmapCount = mipMapCount;
		image.textureTarget = static_cast<TextureTarget>(textureTarget);

		for (auto& vec : image.images)
		{
			vec.resize(mipMapCount);
		}
	}
};

void GenericImage::load(GenericImage* dest, FILE* file)
{
	SerializedGenericImage serialized;
	std::fread(&serialized, sizeof(SerializedGenericImage), 1, file);
	int err;
	if ((err = std::ferror(file)) != 0)
		throw_with_trace(std::runtime_error("Couldn't read from file: Error code =  " + std::to_string(err)));

	serialized.fill(*dest);

	std::fread(dest->pixels.data(), dest->pixels.size(), 1, file);

	if ((err = std::ferror(file)) != 0)
		throw_with_trace(std::runtime_error("Couldn't read from file: Error code =  " + std::to_string(err)));
}

void GenericImage::write(const GenericImage& image, FILE* file)
{
	SerializedGenericImage serialized(image);

	int err;
	std::fwrite(&serialized, sizeof(SerializedGenericImage), 1, file);
	if ((err = std::ferror(file)) != 0)
		throw_with_trace(std::runtime_error("Couldn't write to file: Error code =  " + std::to_string(err)));

	std::fwrite(image.pixels.data(), image.pixels.size(), 1, file);
	if ((err = std::ferror(file)) != 0)
		throw_with_trace(std::runtime_error("Couldn't write to file: Error code =  " + std::to_string(err)));
}


void StoreImage::load(StoreImage* dest, const char* filePath)
{
	FILE* file = nullptr;
	errno_t err;
	if ((err = fopen_s(&file, filePath, "rb")) != 0)
		throw_with_trace(std::runtime_error("Couldn't read from file " + std::string(filePath)));


	SerializedStoreImage serialized;

	std::fread(&serialized, sizeof(SerializedStoreImage), 1, file);

	if (std::ferror(file) != 0)
		throw_with_trace(std::runtime_error("Couldn't read from file " + std::string(filePath)));

	serialized.fill(*dest);

	if (std::ferror(file) != 0)
		throw_with_trace(std::runtime_error("Couldn't read from file " + std::string(filePath)));
	
	for (unsigned i = 0; i < dest->images.size(); ++i)
	{
		for (unsigned j = 0; j < dest->mipmapCount; ++j)
		{
			GenericImage::load(&dest->images[i][j], file);
		}
	}


	fclose(file);
}

void StoreImage::write(const StoreImage& source, const char* filePath)
{
	FILE* file = nullptr;
	errno_t err;
	if ((err = fopen_s(&file, filePath, "w+b")) != 0)
		throw_with_trace(std::runtime_error("Couldn't write to file " + std::string(filePath)));


	SerializedStoreImage serialized(source);
	std::fwrite(&serialized, sizeof(SerializedStoreImage), 1, file);

	for (unsigned i = 0; i < source.images.size(); ++i)
	{
		for (unsigned j = 0; j < source.mipmapCount; ++j)
		{
			GenericImage::write(source.images[i][j], file);
		}
	}

	if (std::ferror(file) != 0)
		throw_with_trace(std::runtime_error("Couldn't write to file " + std::string(filePath)));

	fclose(file);
}

void StoreImage::create(StoreImage* result, unsigned short levels, unsigned short mipMapCountPerLevel, TextureTarget target)
{
	assert(levels > 0);
	assert(mipMapCountPerLevel > 0);

	if (target == TextureTarget::CUBE_MAP)
	{
		levels = 6;
	}

	result->images.resize(levels);
	result->mipmapCount = mipMapCountPerLevel;
	result->textureTarget = target;

	for (auto& vec : result->images)
	{
		vec.resize(result->mipmapCount);
	}
}