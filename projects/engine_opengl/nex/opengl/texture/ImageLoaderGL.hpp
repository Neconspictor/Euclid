#pragma once
#include <glad/glad.h>
#include <string>
#include <nex/logging/LoggingClient.hpp>

struct GenericImageGL
{
	char* pixels;
	unsigned long long bufSize;
	int width, height;
	int components; // of how many components consists a pixel? E.g. 3 for RGB or 4 for RGBA
	GLint format;
	unsigned int numMipmaps;
};

class ImageLoaderGL
{
public:
	explicit ImageLoaderGL();
	GenericImageGL loadImageFromDisc(std::string fileName);

private:
	nex::LoggingClient logClient;

	GenericImageGL loadDDSFile(char* imageData, std::streampos fileSize);
};