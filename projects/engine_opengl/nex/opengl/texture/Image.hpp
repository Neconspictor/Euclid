#pragma once
#include <glad/glad.h>
#include <string>
#include <nex/common/Log.hpp>
#include "nex/util/Memory.hpp"

namespace nex
{
	struct GenericImageGL
	{
		nex::MemGuard pixels;
		unsigned long long bufSize = 0;
		unsigned int width = 0;
		unsigned int height = 0;
		size_t pixelSize = 0; // The byte size of one pixel (all components combined)
		unsigned int components = 0; // of how many components consists a pixel? E.g. 3 for RGB or 4 for RGBA
		GLint format = 0;

		GenericImageGL() = default;
		GenericImageGL(GenericImageGL&& o) noexcept;
		GenericImageGL& operator=(GenericImageGL&& o) = delete;

		GenericImageGL(const GenericImageGL&) = delete;
		GenericImageGL& operator=(const GenericImageGL&) = delete;

		static void load(GenericImageGL* dest, FILE* file);
		static void write(const GenericImageGL& image, FILE* file);
	};

	struct StoreImageGL
	{

		using Image2DArray = nex::GuardArray<nex::GuardArray<GenericImageGL>>;

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
		nex::GuardArray<unsigned short> mipmapCounts; // The number of mipmaps for each side
		unsigned short sideCount = 0; // 6 for cubemaps, arbitrary number for texture arrays, 1 otherwise

		StoreImageGL() = default;
		StoreImageGL(StoreImageGL&& o) noexcept;
		StoreImageGL& operator=(StoreImageGL&& o) = delete;

		StoreImageGL(const StoreImageGL&) = delete;
		StoreImageGL& operator=(const StoreImageGL&) = delete;

		static void load(StoreImageGL* dest, const char* filePath);
		static void write(const StoreImageGL& source, const char* filePath);

		static void create(StoreImageGL* result, unsigned short sideCount, unsigned short mipMapCountPerSide);
	};

	class ImageLoaderGL
	{
	public:
		explicit ImageLoaderGL();
		//GenericImageGL loadImageFromDisc(std::string fileName);

	private:
		nex::Logger m_logger;

		GenericImageGL loadDDSFile(char* imageData, std::streampos fileSize);
	};
}