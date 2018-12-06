#include <nex/texture/Image.hpp>
#include <nex/FileSystem.hpp>
//#include <DDS.h>
#include <boost/interprocess/streams/bufferstream.hpp>
#include <nex/util/ExceptionHandling.hpp>
#include <nex/util/StringUtils.hpp>


using namespace boost::interprocess;

using namespace std;
using namespace nex;

GenericImage::GenericImage(GenericImage&& o) noexcept :
	pixels(std::move(o.pixels)), width(o.width), height(o.height), components(o.components),
	bufSize(o.bufSize), pixelSize(o.pixelSize), format(o.format)
{
}

void GenericImage::load(GenericImage* dest, FILE* file)
{
	std::fread(dest, sizeof(GenericImage), 1, file);
	int err;
	if ((err = std::ferror(file)) != 0)
		throw_with_trace(std::runtime_error("Couldn't read from file: Error code =  " + std::to_string(err)));

	dest->pixels = new char[dest->bufSize];

	std::fread(&*dest->pixels, dest->bufSize, 1, file);

	if ((err = std::ferror(file)) != 0)
		throw_with_trace(std::runtime_error("Couldn't read from file: Error code =  " + std::to_string(err)));
}

void GenericImage::write(const GenericImage& image, FILE* file)
{
	int err;
	std::fwrite(&image, sizeof(GenericImage), 1, file);
	if ((err = std::ferror(file)) != 0)
		throw_with_trace(std::runtime_error("Couldn't write to file: Error code =  " + std::to_string(err)));

	std::fwrite(&*image.pixels, image.bufSize, 1, file);
	if ((err = std::ferror(file)) != 0)
		throw_with_trace(std::runtime_error("Couldn't write to file: Error code =  " + std::to_string(err)));
}

StoreImage::StoreImage(StoreImage&& o) noexcept :
images(std::move(o.images)), mipmapCounts(std::move(o.mipmapCounts)), sideCount(o.sideCount)
{
	o.sideCount = 0;
}


void StoreImage::load(StoreImage* dest, const char* filePath)
{
	FILE* file = nullptr;
	errno_t err;
	if ((err = fopen_s(&file, filePath, "rb")) != 0)
		throw_with_trace(std::runtime_error("Couldn't read from file " + std::string(filePath)));

	std::fread(dest, sizeof(StoreImage), 1, file);

	if (std::ferror(file) != 0)
		throw_with_trace(std::runtime_error("Couldn't read from file " + std::string(filePath)));

	dest->mipmapCounts = new unsigned short[dest->sideCount];
	std::fread(dest->mipmapCounts.get(), sizeof(unsigned short) * dest->sideCount, 1, file);

	if (std::ferror(file) != 0)
		throw_with_trace(std::runtime_error("Couldn't read from file " + std::string(filePath)));
	
	dest->images = new GuardArray<GenericImage>[dest->sideCount];

	
	for (unsigned i = 0; i < dest->sideCount; ++i)
	{
		dest->images[i] = new GenericImage[dest->mipmapCounts[i]];

		for (unsigned j = 0; j < dest->mipmapCounts[i]; ++j)
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

	std::fwrite(&source, sizeof(StoreImage), 1, file);
	std::fwrite(source.mipmapCounts.get(), sizeof(unsigned short), source.sideCount, file);

	for (unsigned i = 0; i < source.sideCount; ++i)
	{
		auto& array = source.images[i];
		for (unsigned j = 0; j < source.mipmapCounts[i]; ++j)
		{
			GenericImage::write(array[j], file);
		}
	}

	if (std::ferror(file) != 0)
		throw_with_trace(std::runtime_error("Couldn't write to file " + std::string(filePath)));

	fclose(file);
}

void StoreImage::create(StoreImage* result, unsigned short sideCount, unsigned short mipMapCountPerSide)
{
	assert(sideCount > 0);
	assert(mipMapCountPerSide > 0);

	result->sideCount = sideCount;
	result->mipmapCounts = new unsigned short[result->sideCount];
	for (unsigned short i = 0; i < result->sideCount; ++i)
	{
		result->mipmapCounts[i] = mipMapCountPerSide;
	}
	
	result->images = new GuardArray<GenericImage>[result->sideCount];

	for (unsigned short i = 0; i < result->sideCount; ++i)
	{
		result->images[i] = new GenericImage[result->mipmapCounts[i]];
	}
}