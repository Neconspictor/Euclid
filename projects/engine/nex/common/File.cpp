#include <nex/common/File.hpp>
#include "nex/util/ExceptionHandling.hpp"
#include "Log.hpp"

nex::BinStream::BinStream(size_t bufferSize) : std::fstream(), mBuffer(bufferSize)
{
	rdbuf()->pubsetbuf(mBuffer.data(), mBuffer.size());
	exceptions(std::ofstream::failbit | std::ofstream::badbit);
}

nex::BinStream::~BinStream()
{
	Logger logger;
	LOG(logger, Info) << "Calling ~BinStream";
	if (is_open())
	{
		LOG(logger, Info) << "closing BinStream...";
		flush();
		close();
	}
}

void nex::BinStream::open(const char* filePath, std::ios_base::openmode mode)
{
	try
	{
		std::fstream::open(filePath, mode | std::ios::binary);
	}
	catch (std::exception& e)
	{
		throw_with_trace(e);
	}
}

void nex::BinStream::open(std::filesystem::path& path, std::ios_base::openmode mode)
{
	try
	{
		std::fstream::open(path, mode | std::ios::binary);
	}
	catch (std::exception& e)
	{
		throw_with_trace(e);
	}
}

void nex::BinStream::test()
{
	size_t bytes = 8192;
	{
		BinStream file;
		file.open("test.bin", std::ios::out | std::ios::trunc);
		file << bytes;
	}

	{
		BinStream file4;
		file4.open("test.bin", std::ios::in);
		bytes = 0;
		file4 >> bytes;
	}

	std::cout << "bytes = " << bytes << std::endl;
}