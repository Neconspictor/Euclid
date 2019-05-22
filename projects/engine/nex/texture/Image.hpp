#pragma once
#include "nex/util/Memory.hpp"
#include <vector>
#include <nex/texture/TextureSamplerData.hpp>
#include <nex/exception/ResourceLoadException.hpp>


namespace nex
{
	struct GenericImage
	{
		std::vector<char> pixels;
		unsigned width = 0;
		unsigned height = 0;
		size_t pixelSize = 0; // The byte size of one pixel (all components combined)
		unsigned components = 0; // of how many components consists a pixel? E.g. 3 for RGB or 4 for RGBA
		unsigned format = 0;

		GenericImage() = default;
		~GenericImage() = default;
		GenericImage(GenericImage&& o) = default;
		GenericImage& operator=(GenericImage&& o) = delete;

		GenericImage(const GenericImage&) = delete;
		GenericImage& operator=(const GenericImage&) = delete;

		static void load(GenericImage* dest, FILE* file);
		static void write(const GenericImage& image, FILE* file);
	};


	class ImageFactory
	{
	public:

		struct ImageResource
		{
			ImageResource() noexcept;
			~ImageResource() noexcept;
			ImageResource(ImageResource&& o) noexcept;
			ImageResource& operator=(ImageResource&& o) noexcept;
			ImageResource(const ImageResource&) = delete;
			ImageResource& operator=(const ImageResource&) = delete;

			int width;
			int height;
			int channels;
			int pixelSize;
			int stride;

			/**
			 * The image data. 
			 * Note: Memory is managed by this class!
			 */
			void* data;
		};


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
		 * @param flipY : Should the y axis be flipped?
		 * 
		 * @throws nex::ResourceLoadException : if the image couldn't be loaded.
		 */
		static ImageResource loadHDR(const char* filePath, bool flipY, int desiredChannels = 0);

		/**
		 * @param desiredChannels : the number of channels the image should have. Specify zero, if the channels should be examined automatically.
		 * @param flipY : Should the y axis be flipped?
		 * 
		 * @throws nex::ResourceLoadException : if the image couldn't be loaded.
		 */
		static ImageResource loadNonHDR(const char* filePath, bool flipY, int desiredChannels = 0);
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
		StoreImage& operator=(StoreImage&& o) = delete;

		StoreImage(const StoreImage&) = delete;
		StoreImage& operator=(const StoreImage&) = delete;

		static void load(StoreImage* dest, const char* filePath);
		static void write(const StoreImage& source, const char* filePath);

		static void create(StoreImage* result, unsigned short levels, unsigned short mipMapCountPerLevel, TextureTarget target);
	};
}
