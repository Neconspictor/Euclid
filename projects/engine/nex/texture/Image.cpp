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

nex::BinStream& nex::operator<<(nex::BinStream& out, const GenericImage& image)
{
	out << image.pixels;
	out << image.width;
	out << image.height;
	out << image.pixelSize;
	out << image.channels;
	out << image.format;
	out << image.stride;

	return out;
}

nex::BinStream& nex::operator>>(nex::BinStream& in, GenericImage& image)
{
	in >> image.pixels;
	in >> image.width;
	in >> image.height;
	in >> image.pixelSize;
	in >> image.channels;
	in >> image.format;
	in >> image.stride;

	return in;
}

ImageResource::ImageResource() noexcept : data(nullptr), bytes(0)
{
}

ImageResource::~ImageResource() noexcept
{
	if (data)stbi_image_free(data);
	data = nullptr;
	bytes = 0;
}

ImageResource::ImageResource(ImageResource&& o) noexcept : data(nullptr), bytes(0)
{
	*this = std::move(o);
}

ImageResource& ImageResource::operator=(ImageResource&& o) noexcept
{
	std::swap(data, o.data);
	std::swap(bytes, o.bytes);
	return *this;
}

nex::BinStream& nex::operator<<(nex::BinStream& out, const ImageResource& resource)
{
	out << resource.bytes;
	auto* ptr = (const char*)resource.data;
	out.write(ptr, resource.bytes);
	return out;
}

PixelVariant::PixelVariant() : std::variant<std::vector<char>, ImageResource>()
{
}

const void* PixelVariant::getPixels() const
{
	return getPixelsMutable();
}

void* PixelVariant::getPixels()
{
	return getPixelsMutable();
}

size_t PixelVariant::getBufferSize() const
{
	if (std::holds_alternative<std::vector<char>>(*this))
	{
		return std::get<std::vector<char>>(*this).size();
	}
	else if (std::holds_alternative<ImageResource>(*this))
	{
		return std::get<ImageResource>(*this).bytes;
	}

	/**
	 * no pixel data set, yet.
	 */
	return 0;
}

PixelVariant& PixelVariant::operator=(ImageResource&& resource)
{
	std::variant<std::vector<char>, ImageResource>::operator=(std::move(resource));
	return *this;
}

PixelVariant& PixelVariant::operator=(std::vector<char>&& vec)
{
	std::variant<std::vector<char>, ImageResource>::operator=(std::move(vec));
	return *this;
}

void* PixelVariant::getPixelsMutable() const
{
	if (std::holds_alternative<std::vector<char>>(*this))
	{
		return (void*)std::get<std::vector<char>>(*this).data();
	}
	else if (std::holds_alternative<ImageResource>(*this))
	{
		return std::get<ImageResource>(*this).data;
	}

	/**
	 * no pixel data set, yet.
	 */
	return nullptr;
}

nex::BinStream& nex::operator<<(nex::BinStream& out, const PixelVariant& variant)
{
	if (std::holds_alternative<std::vector<char>>(variant))
	{
		const auto& vec = std::get<std::vector<char>>(variant);
		out << vec;
	}
	else if (std::holds_alternative<ImageResource>(variant))
	{
		const auto& resource = std::get<ImageResource>(variant);
		out << resource;
	} else
	{
		// No pixels stored;
		size_t count = 0;
		out << count;
	}

	return out;
}

nex::BinStream& nex::operator>>(nex::BinStream& in, PixelVariant& variant)
{
	// we always deserialize the data to a std::vector<char>
	variant = std::vector<char>();
	auto& vec = std::get<std::vector<char>>(variant);
	in >> vec;

	return in;
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
	stbi_write_hdr(filePath, imageData.width, imageData.height, imageData.channels, (const float*)imageData.pixels.getPixels());
}

nex::GenericImage ImageFactory::loadHDR(const char* filePath, bool flipY, int desiredChannels)
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

	if (desiredChannels != 0) channels = desiredChannels;
	const size_t pixelSize = channels * sizeof(float);

	GenericImage image;
	ImageResource resource;
	resource.data = rawData;
	resource.bytes = width * height * pixelSize;

	image.width = width;
	image.height = height;
	image.channels = channels;
	image.pixelSize = pixelSize;
	image.stride = width * image.pixelSize;
	image.pixels = std::move(resource);

	return image;
}

GenericImage ImageFactory::loadNonHDR(const char* filePath, bool flipY, int desiredChannels)
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

	if (desiredChannels != 0) channels = desiredChannels;
	const size_t pixelSize = channels * sizeof(unsigned char);

	GenericImage image;
	ImageResource resource;
	resource.data = rawData;
	resource.bytes = width * height * pixelSize;

	image.width = width;
	image.height = height;
	image.channels = channels;
	image.pixelSize = pixelSize;
	image.stride = width * image.pixelSize;
	image.pixels = std::move(resource);

	return image;
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

nex::BinStream& nex::operator<<(nex::BinStream& out, const StoreImage& image)
{
	out << image.images;
	out << image.mipmapCount;
	out << image.textureTarget;

	return out;
}

nex::BinStream& nex::operator>>(nex::BinStream& in, StoreImage& image)
{
	in >> image.images;
	in >> image.mipmapCount;
	in >> image.textureTarget;

	return in;
}