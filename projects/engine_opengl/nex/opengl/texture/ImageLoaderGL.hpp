#pragma once
#include <glad/glad.h>
#include <string>
#include <nex/common/Log.hpp>

struct GenericImageGL
{
	char* pixels = nullptr;
	unsigned long long bufSize = 0;
	int width = 0;
	int height = 0;
	unsigned char pixelSize = 0; // The byte size of one pixel (all components combined)
	int components = 0; // of how many components consists a pixel? E.g. 3 for RGB or 4 for RGBA
	GLint format = 0;
	unsigned int numMipmaps = 0;
};

class ImageLoaderGL
{
public:
	explicit ImageLoaderGL();
	GenericImageGL loadImageFromDisc(std::string fileName);

private:
	nex::Logger m_logger;

	GenericImageGL loadDDSFile(char* imageData, std::streampos fileSize);
};