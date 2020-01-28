#include <nex/texture/Image.hpp>
#include <nex/resource/FileSystem.hpp>
//#include <DDS.h>
#include <boost/interprocess/streams/bufferstream.hpp>
#include <nex/util/ExceptionHandling.hpp>
#include <nex/exception/ResourceLoadException.hpp>
#include <nex/util/StringUtils.hpp>
#include <nex/texture/Texture.hpp>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#define STBI_MSC_SECURE_CRT
#include <stb/stb_image_write.h>


#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>
#include "nex/common/Log.hpp"


using namespace boost::interprocess;

using namespace std;
using namespace nex;

bool nex::ImageFactory::mFlipY = false;

nex::BinStream& nex::operator<<(nex::BinStream& out, const GenericImage& image)
{
	out << image.pixels;
	out << image.desc;

	return out;
}

nex::BinStream& nex::operator>>(nex::BinStream& in, GenericImage& image)
{
	in >> image.pixels;
	in >> image.desc;

	return in;
}

void ImageFactory::init(bool flipY)
{
	mFlipY = flipY;
	stbi_set_flip_vertically_on_load(mFlipY);
}

bool nex::ImageFactory::isYFlipped()
{
	return mFlipY;
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

nex::GenericImage ImageFactory::loadFloat(const std::filesystem::path& filePath, bool flipY, int desiredChannels)
{
	return loadGeneric(filePath,
		flipY,
		PixelDataType::FLOAT,
		1,
		desiredChannels,
		reinterpret_cast<GenericLoader*>(stbi_loadf));
}

GenericImage ImageFactory::loadUByte(const std::filesystem::path& filePath, bool flipY, int desiredChannels)
{
	return loadGeneric(filePath, 
		flipY, 
		PixelDataType::UBYTE, 
		1, 
		desiredChannels, 
		reinterpret_cast<GenericLoader*>(stbi_load));
}

GenericImage nex::ImageFactory::loadGeneric(const std::filesystem::path& filePath, 
	bool flipY, 
	PixelDataType pixelDataType, 
	int rowAlignment, 
	int desiredChannels, 
	GenericLoader* loader)
{
	int width;
	int height;
	int channels;

	stbi_set_flip_vertically_on_load(flipY);
	void* rawData = loader(filePath, &width, &height, &channels, desiredChannels);

	if (!rawData) {
		Logger logger("ImageFactory");
		LOG(logger, Fault) << "Couldn't load image file: " << filePath << std::endl;
		stringstream ss;
		ss << "ImageFactory::loadGeneric: Couldn't load image file: " << filePath;
		throw_with_trace(ResourceLoadException(ss.str()));
	}

	if (desiredChannels != 0) channels = desiredChannels;

	GenericImage image;
	auto& desc = image.desc;

	desc.width = width;
	desc.height = height;
	desc.depth = 1;
	desc.colorspace = mapToRGBASubColorSpace(channels);
	desc.pixelDataType = pixelDataType;
	desc.rowByteAlignmnet = rowAlignment;

	const size_t pixelSize = desc.calcPixelByteSize();

	ImageResource resource;
	resource.data = rawData;
	resource.bytes = desc.width * desc.height * pixelSize;
	image.pixels = std::move(resource);

	return image;
}

ColorSpace nex::ImageFactory::mapToRGBASubColorSpace(int numChannels)
{
	switch (numChannels) {
	case 1: return ColorSpace::R;
	case 2: return ColorSpace::RG;
	case 3: return ColorSpace::RGB;
	case 4: return ColorSpace::RGBA;
	default: nex::throw_with_trace(std::runtime_error("Not supported channel count: " + std::to_string(numChannels)));
	}

	// Only for disabling compile warnings
	return ColorSpace::RGBA;
}

void StoreImage::create(StoreImage* result, unsigned short levels, unsigned short mipMapCountPerLevel, TextureTarget target, glm::uvec2&& tileCount)
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
	result->tileCount = std::move(tileCount);

	for (auto& vec : result->images)
	{
		vec.resize(result->mipmapCount);
	}
}

StoreImage nex::StoreImage::create(Texture * texture, PixelDataType pixelDataType, bool allMipMaps, unsigned mipMapStart, unsigned mipmapCount, unsigned rowByteAlignment)
{
	StoreImage store;

	const auto target = texture->getTarget();
	const auto& data = texture->getTextureData();

	if (target == TextureTarget::TEXTURE1D
		|| target == TextureTarget::TEXTURE1D_ARRAY
		|| target == TextureTarget::CUBE_MAP_ARRAY
		|| target == TextureTarget::TEXTURE2D_ARRAY) {

		throw std::invalid_argument("nex::StoreImage::create: target " + std::to_string((unsigned)target) + " not supported, yet!");
	}

	unsigned imageCount = texture->getDepth();
	if (target == TextureTarget::CUBE_MAP)
		imageCount = ((CubeMap*)texture)->getLayerFaces();

	if (allMipMaps) {
		auto maxMipMapCount = texture->calcMipMapCount(texture->getLevelZeroMipMapTextureSize());
		mipmapCount = data.lodMaxLevel - data.lodBaseLevel + 1;
		mipmapCount = (maxMipMapCount < mipmapCount) ? maxMipMapCount : mipmapCount;
		mipMapStart = data.lodBaseLevel;
	}

	StoreImage::create(&store, imageCount, mipmapCount, target, glm::uvec2(texture->getTileCount()));
	readback(store, texture, mipMapStart, pixelDataType, rowByteAlignment);

	return store;
}

void nex::StoreImage::fill(CubeMapArray * texture, const StoreImage & store, unsigned arrayIndex)
{
	auto layerFaceStartIndex = CubeMapArray::getLayerFaceIndex(arrayIndex);


	for (unsigned mipmap = 0; mipmap < store.mipmapCount; ++mipmap) {
		for (unsigned i = 0; i < store.images.size(); ++i) {

			const auto& image = store.images[i][mipmap];

			TextureTransferDesc transferDesc;
			transferDesc.data = const_cast<void*>(image.pixels.getPixels());
			transferDesc.dataByteSize = image.pixels.getBufferSize();
			transferDesc.mipMapLevel = mipmap;
			transferDesc.xOffset = 0;
			transferDesc.yOffset = 0;
			transferDesc.zOffset = layerFaceStartIndex + i;
			transferDesc.imageDesc = image.desc;
			transferDesc.imageDesc.depth = 1;
			texture->upload(transferDesc);
		}
	}

	texture->setTileCount(store.tileCount);
}

void nex::StoreImage::readback(nex::StoreImage & store, nex::Texture * texture, unsigned mipMapStart, PixelDataType pixelDataType, unsigned rowByteAlignmnet)
{
	const auto& data = texture->getTextureData();

	const auto colorspace = nex::getColorSpace(data.internalFormat);
	const auto components = getComponents(colorspace);

	store.textureTarget = texture->getTarget();
	store.tileCount = texture->getTileCount();

	for (auto level = mipMapStart; level < mipMapStart + store.mipmapCount; ++level)
	{

		

		// readback the mipmap level of the cubemap
		const auto pixelDataSize = sizeof(float) * components;
		unsigned mipMapDivisor = std::pow(2, level);
		const auto width = texture->getWidth() / mipMapDivisor;
		const auto height = texture->getHeight() / mipMapDivisor;
		const auto sideSlice = width * height * pixelDataSize;
		std::vector<char> pixels(sideSlice * texture->getDepth());

		TextureTransferDesc desc;
		auto& imageDesc = desc.imageDesc;
		desc.data = pixels.data();
		desc.dataByteSize = pixels.size();
		desc.mipMapLevel = level;
		desc.xOffset = desc.yOffset = desc.zOffset = 0;
		imageDesc.colorspace = colorspace;
		imageDesc.pixelDataType = pixelDataType;
		imageDesc.rowByteAlignmnet = rowByteAlignmnet;
		imageDesc.width = width;
		imageDesc.height = height;
		imageDesc.depth = texture->getDepth(); // all six cubemap sides

		texture->readback(desc);
			

		for (unsigned side = 0; side < store.images.size(); ++side)
		{
			auto& image = store.images[side][level];
			image.desc.width = width;
			image.desc.height = height;
			image.desc.depth = 1;
			image.desc.colorspace = colorspace;
			image.desc.pixelDataType = pixelDataType;
			image.desc.rowByteAlignmnet = rowByteAlignmnet;
			auto bufSize = sideSlice;
			image.pixels = std::vector<char>(bufSize);

			memcpy_s(image.pixels.getPixels(), bufSize, pixels.data() + side * sideSlice, sideSlice);
		}
	}
}

nex::BinStream& nex::operator<<(nex::BinStream& out, const StoreImage& image)
{
	out << image.images;
	out << image.mipmapCount;
	out << image.textureTarget;
	out << image.tileCount;

	return out;
}

nex::BinStream& nex::operator>>(nex::BinStream& in, StoreImage& image)
{
	in >> image.images;
	in >> image.mipmapCount;
	in >> image.textureTarget;
	in >> image.tileCount;

	return in;
}

unsigned nex::ImageDesc::calcPixelByteSize() const
{
	auto numComponentsPerDataType = getPixelDataTypePackedComponentsCount(pixelDataType);
	auto dataTypeByteSize = getPixelDataTypeByteSize(pixelDataType);
	auto channelCount = getComponents(colorspace);
	unsigned dataTypeCount = std::ceilf(channelCount / (float)numComponentsPerDataType);
	return dataTypeCount * dataTypeByteSize;
}