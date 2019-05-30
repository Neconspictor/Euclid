#include <nex/common/File.hpp>
#include "nex/util/ExceptionHandling.hpp"
#include "Log.hpp"

nex::BinStream::BinStream(size_t bufferSize) : std::fstream(), mBuffer(bufferSize)
{
	rdbuf()->pubsetbuf(mBuffer.data(), mBuffer.size());
	exceptions(std::ofstream::failbit | std::ofstream::badbit);
}

nex::BinStream::~BinStream() noexcept
{
	/*Logger logger;

	if (fail())
	{
		LOG(logger, Info) << "fail state active : " << mFile.generic_string();
	}

	if (eof())
	{
		LOG(logger, Info) << "eof state active : " << mFile.generic_string();
	}

	auto state = rdstate();   // get state
	state &= ~std::ios_base::failbit;
	state &=  ~std::ios_base::badbit;
	state &= ~std::ios_base::eofbit;
	clear(state);
	
	LOG(logger, Info) << "cleared BinStream : " << mFile.generic_string();

	if (fail())
	{
		LOG(logger, Info) << "fail state still active : " << mFile.generic_string();
	}

	if (eof())
	{
		LOG(logger, Info) << "eof state still active : " << mFile.generic_string();
	}*/


}

void nex::BinStream::open(const char* filePath, std::ios_base::openmode mode)
{
	try
	{
		mFile = filePath;
		Logger logger;
		LOG(logger, Info) << "open BinStream : " << mFile.generic_string();
		std::fstream::open(filePath, mode | std::ios::binary);
		LOG(logger, Info) << "opened BinStream : " << mFile.generic_string();
	}
	catch (std::exception& e)
	{
		throw_with_trace(e);
	}
}

void nex::BinStream::open(const std::filesystem::path& path, std::ios_base::openmode mode)
{
	try
	{
		mFile = path;
		Logger logger;
		LOG(logger, Info) << "open BinStream : " << mFile.generic_string();
		std::fstream::open(path, mode | std::ios::binary);
		LOG(logger, Info) << "opened BinStream : " << mFile.generic_string();
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