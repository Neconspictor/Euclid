#pragma once
#include <vector>
#include <nex/texture/TextureSamplerData.hpp>
#include <variant>


namespace nex
{
	class BinStream;
	class Texture;

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
		unsigned width = 0;
		unsigned height = 0;
		size_t pixelSize = 0; // The byte size of one pixel (all components combined)
		unsigned channels = 0; // of how many components consists a pixel? E.g. 3 for RGB or 4 for RGBA
		unsigned format = 0;
		unsigned stride = 0;

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
		 * @param stride : byte size of one line (== width * pixel-size)
		 */
		static void writeToPNG(const char* filePath, 
				const char* image, 
				size_t width, 
				size_t height, 
				size_t components, 
				size_t stride, 
				bool flipY);

		/**
		 * Note: Alpha-channel (if present) will be ignored
		 */
		static void writeHDR(const nex::GenericImage& imageData, const char* filePath, bool flipY);

		/**
		 * @param desiredChannels : the number of channels the image should have. Specify zero, if the channels should be examined automatically.
		 * 
		 * @throws nex::ResourceLoadException : if the image couldn't be loaded.
		 */
		static GenericImage loadHDR(const char* filePath, int desiredChannels = 0);

		/**
		 * @param desiredChannels : the number of channels the image should have. Specify zero, if the channels should be examined automatically.
		 * 
		 * @throws nex::ResourceLoadException : if the image couldn't be loaded.
		 */
		static GenericImage loadNonHDR(const char* filePath, int desiredChannels = 0);

	private:
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
		TextureTarget textureTarget = TextureTarget::TEXTURE2D;

		StoreImage() = default;
		StoreImage(StoreImage&& o) noexcept = default;
		StoreImage& operator=(StoreImage&& o) = default;

		StoreImage(const StoreImage&) = delete;
		StoreImage& operator=(const StoreImage&) = delete;

		static void create(StoreImage* result, unsigned short levels, unsigned short mipMapCountPerLevel, TextureTarget target);

		/**
		 * @param allMipMaps :  Should all mipmaps be stored?
		 * @param mipMapStart : The start of mipmaps to be stored. Is ignored, if allMipMaps is set to true.
		 * @param mipmapCount : The number of mipmaps to be stored. Is ignored if allMipMaps is set to true.
		 */
		static StoreImage create(Texture* texture, bool allMipMaps = true, unsigned mipMapStart = 0, unsigned mipmapCount = 1);

	private:
		static void readback(StoreImage& store, Texture* texture, unsigned mipMapStart);
	};

	nex::BinStream& operator<<(nex::BinStream& out, const StoreImage& image);
	nex::BinStream& operator>>(nex::BinStream& in, StoreImage& image);
}