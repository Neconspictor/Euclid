#include <nex/texture/Image.hpp>
#include <nex/FileSystem.hpp>
//#include <DDS.h>
#include <boost/interprocess/streams/bufferstream.hpp>
#include <nex/util/ExceptionHandling.hpp>
#include <nex/exception/ResourceLoadException.hpp>
#include <nex/util/StringUtils.hpp>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#define STBI_MSC_SECURE_CRT
#include <stb/stb_image_write.h>


#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>
#include "nex/common/Log.hpp"


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

ImageFactory::ImageResource::ImageResource() noexcept : width(0), height(0), channels(0), pixelSize(0), stride(0), data(nullptr)
{
}

ImageFactory::ImageResource::~ImageResource() noexcept
{
	if (data)stbi_image_free(data);
}

ImageFactory::ImageResource::ImageResource(ImageResource&& o) noexcept : width(o.width), height(o.height),  channels(o.channels),
pixelSize(o.pixelSize), stride(o.stride), data(o.data)
{
	o.data = nullptr;
}

ImageFactory::ImageResource& ImageFactory::ImageResource::operator=(ImageResource&& o) noexcept
{
	if (this == &o) return *this;

	width = o.width;
	height = o.height;
	channels = o.channels;
	pixelSize = o.pixelSize;
	stride = o.stride;
	data = o.data;

	o.data = nullptr;

	return *this;
}

void ImageFactory::writeToPNG(const char* filePath, const char* image, size_t width, size_t height, size_t components,
	size_t stride, bool flipY)
{
	stbi__flip_vertically_on_write = flipY;
	stbi_write_png(filePath, width, height, components, image, stride);
}

void ImageFactory::writeHDR(const nex::GenericImage& imageData, const char* filePath, bool flipY)
{
	stbi__flip_vertically_on_write = flipY;
	stbi_write_hdr(filePath, imageData.width, imageData.height, imageData.components, (float*)imageData.pixels.data());
}

nex::ImageFactory::ImageResource ImageFactory::loadHDR(const char* filePath, bool flipY, int desiredChannels)
{
	stbi_set_flip_vertically_on_load(flipY);

	int width; 
	int height; 
	int channels;

	float *rawData = stbi_loadf(filePath, &width, &height, &channels, desiredChannels);

	if (!rawData) {
		Logger logger("ImageFactory");
		LOG(logger, Fault) << "Couldn't load image file: " << filePath << std::endl;
		stringstream ss;
		ss << "ImageFactory::loadHDR: Couldn't load image file: " << filePath;
		throw_with_trace(ResourceLoadException(ss.str()));
	}

	ImageResource resource;
	resource.width = width;
	resource.height = height;
	resource.channels = channels;
	resource.pixelSize = channels * sizeof(float);
	resource.stride = width * resource.pixelSize;
	resource.data = rawData;

	return resource;
}

ImageFactory::ImageResource ImageFactory::loadNonHDR(const char* filePath, bool flipY, int desiredChannels)
{
	stbi_set_flip_vertically_on_load(flipY);

	int width;
	int height;
	int channels;

	unsigned char* rawData = stbi_load(filePath, &width, &height, &channels, desiredChannels);

	if (!rawData) {
		Logger logger("ImageFactory");
		LOG(logger, Fault) << "Couldn't load image file: " << filePath << std::endl;
		stringstream ss;
		ss << "ImageFactory::loadHDR: Couldn't load image file: " << filePath;
		throw_with_trace(ResourceLoadException(ss.str()));
	}

	ImageResource resource;
	resource.width = width;
	resource.height = height;
	resource.channels = channels;
	resource.pixelSize = channels * sizeof(float);
	resource.stride = width * resource.pixelSize;
	resource.data = rawData;

	return resource;
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