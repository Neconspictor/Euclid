#pragma once
#include <vector>
#include <nex/texture/TextureSamplerData.hpp>
#include <variant>
#include <filesystem>


namespace nex
{
	class BinStream;
	class Texture;
	class CubeMapArray;


	struct ImageDesc {
		unsigned width = 0;
		unsigned height = 0;
		unsigned depth = 0;
		unsigned rowByteAlignmnet = 1;
		PixelDataType pixelDataType = PixelDataType::UBYTE;
		ColorSpace colorspace = ColorSpace::RGBA;

		unsigned calcPixelByteSize() const;
	};

	struct ImageResource
	{
		ImageResource() noexcept;
		~ImageResource() noexcept;
		ImageResource(ImageResource&& o) noexcept;
		ImageResource& operator=(ImageResource&& o) noexcept;
		ImageResource(const ImageResource&) = delete;
		ImageResource& operator=(const ImageResource&) = delete;

		/**
		 * The image data.
		 * Note: Memory is managed by this class!
		 */
		void* data;
		size_t bytes;
	};

	/**
	 * Note: Serialization will be equal to std::vector<char>
	 * Note: We define no deserialization, as the memory is managed by external code.
	 *       If you wish to deserialize an image resource, use std::vector<char>.
	 */
	nex::BinStream& operator<<(nex::BinStream& out, const ImageResource& resource);


	class PixelVariant : public std::variant<std::vector<char>, ImageResource>
	{
	public:
		PixelVariant();

		const void* getPixels() const;
		void* getPixels();
		size_t getBufferSize() const;

		PixelVariant& operator=(ImageResource&& resource);
		PixelVariant& operator=(std::vector<char>&& vec);

	private:
		void* getPixelsMutable() const;
	};

	nex::BinStream& operator<<(nex::BinStream& out, const PixelVariant& variant);
	nex::BinStream& operator>>(nex::BinStream& in, PixelVariant& variant);


	struct GenericImage
	{
		PixelVariant pixels;
		ImageDesc desc;

		GenericImage() = default;
		~GenericImage() = default;
		GenericImage(GenericImage&& o) = default;
		GenericImage& operator=(GenericImage&& o) = default;

		GenericImage(const GenericImage&) = delete;
		GenericImage& operator=(const GenericImage&) = delete;
	};

	nex::BinStream& operator<<(nex::BinStream& out, const GenericImage& image);
	nex::BinStream& operator>>(nex::BinStream& in, GenericImage& image);


	class ImageFactory
	{
	public:

		/**
		 * @param flipY : Should the y axis be flipped?
		 */
		static void init(bool flipY);

		static bool isYFlipped();

		/**
		 * @param desiredChannels : the number of channels the image should have. Specify zero, if the channels should be examined automatically.
		 * 
		 * @throws nex::ResourceLoadException : if the image couldn't be loaded.
		 */
		static GenericImage loadFloat(const std::filesystem::path& filePath, bool isSRGB, bool flipY = true, int desiredChannels = 0);

		/**
		 * @param desiredChannels : the number of channels the image should have. Specify zero, if the channels should be examined automatically.
		 * 
		 * @throws nex::ResourceLoadException : if the image couldn't be loaded.
		 */
		static GenericImage loadUByte(const std::filesystem::path&, bool isSRGB, bool flipY = true, int desiredChannels = 0);

		static GenericImage loadUByte(const unsigned char* data, int dataSize, bool isSRGB, bool flipY = true, int desiredChannels = 0);


	private:

		using GenericLoader = std::function<void*(const std::filesystem::path & filePath, int* width, int* height, int* channels, int desiredChannels)>;

		static GenericImage loadGeneric(const std::filesystem::path& filePath, 
			bool flipY, 
			PixelDataType pixelDataType,
			int rowAlignment,
			int desiredChannels, 
			bool isSRGB,
			GenericLoader* loader);

		static ColorSpace mapToRGBASubColorSpace(int numChannels);

		static bool mFlipY;
	};

	struct StoreImage
	{

		using Image2DArray = std::vector<std::vector<GenericImage>>;

		/**
		 * Memory layout for images:
		 *
		 * Definitions:
		 *  - base image: The first image in the mipmap chain. This is the
		 *
		 * images[0] points to all mipmap images of the first side (if present)
		 *	 - images[0][0] is the base image of the first side
		 *	 - images[0][mipmapCounts[0] - 1] is the highest mipmap level image of the first side
		 *
		 * images[sideCount-1] points to all mipmap images of the last side (only if sideCount > 0)
		 */
		Image2DArray images; // a pointer to an array of sideCount base images. With base image 
		unsigned short mipmapCount; // The number of mipmaps for each side
		TextureTarget textureTarget = TextureTarget::TEXTURE2D; // Texture target
		glm::uvec2 tileCount; // tile count for texture atlases			

		StoreImage() = default;
		StoreImage(StoreImage&& o) noexcept = default;
		StoreImage& operator=(StoreImage&& o) = default;

		StoreImage(const StoreImage&) = delete;
		StoreImage& operator=(const StoreImage&) = delete;

		static void create(StoreImage* result, unsigned short levels, unsigned short mipMapCountPerLevel, TextureTarget target, glm::uvec2&& tileCount);

		/**
		 * @param allMipMaps :  Should all mipmaps be stored?
		 * @param mipMapStart : The start of mipmaps to be stored. Is ignored, if allMipMaps is set to true.
		 * @param mipmapCount : The number of mipmaps to be stored. Is ignored if allMipMaps is set to true.
		 */
		static StoreImage create(Texture* texture, 
			PixelDataType pixelDataType, 
			bool allMipMaps = true, 
			unsigned mipMapStart = 0, 
			unsigned mipmapCount = 1, 
			unsigned rowByteAlignment = 1);

		/**
		 * Fills a cubemap at a given index from a store image.
		 */
		static void fill(CubeMapArray* texture, const StoreImage& store, unsigned arrayIndex);

	private:
		static void readback(StoreImage& store, Texture* texture, unsigned mipMapStart, PixelDataType pixelDataType, unsigned rowByteAlignmnet);
	};

	nex::BinStream& operator<<(nex::BinStream& out, const StoreImage& image);
	nex::BinStream& operator>>(nex::BinStream& in, StoreImage& image);
}