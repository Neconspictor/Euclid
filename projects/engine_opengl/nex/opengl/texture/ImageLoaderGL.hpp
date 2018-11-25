#pragma once
#include <glad/glad.h>
#include <string>
#include <nex/common/Log.hpp>

struct GenericImageGL
{
	char* pixels = nullptr;
	unsigned long long bufSize = 0;
	unsigned int width = 0;
	unsigned int height = 0;
	size_t pixelSize = 0; // The byte size of one pixel (all components combined)
	unsigned int components = 0; // of how many components consists a pixel? E.g. 3 for RGB or 4 for RGBA
	GLint format = 0;

	GenericImageGL() = default;
	GenericImageGL(GenericImageGL&& o) noexcept;
	GenericImageGL& operator=(GenericImageGL&& o) noexcept;

	virtual ~GenericImageGL();

	GenericImageGL(const GenericImageGL&) = delete;
	GenericImageGL& operator=(const GenericImageGL&) = delete;
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