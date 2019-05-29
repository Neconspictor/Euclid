#include <nex/common/File.hpp>
#include "nex/util/ExceptionHandling.hpp"
#include "Log.hpp"

nex::File::~File()
{
	Logger logger;
	LOG(logger, Info) << "Calling ~File";

	if (mStream.is_open())
	{
		LOG(logger, Info) << "closing file stream...";
		mStream.flush();
		mStream.close();
	}
}

std::fstream& nex::File::open(const char* filePath, std::ios_base::openmode mode)
{
	return open(std::filesystem::path(filePath), mode);
}

std::fstream&  nex::File::open(std::filesystem::path& path, std::ios_base::openmode mode)
{
	try
	{
		mStream.open(path, mode);
	} catch(std::exception& e)
	{
		throw_with_trace(e);
	}

	return get();
}

nex::File::File(size_t bufferSize) : mBuffer(bufferSize)
{
	mStream.rdbuf()->pubsetbuf(mBuffer.data(), mBuffer.size());
	mStream.exceptions(std::ofstream::failbit | std::ofstream::badbit);
}

std::fstream& nex::File::get()
{
	return mStream;
}

std::fstream& nex::File::operator*()
{
	return mStream;
}

const std::fstream& nex::File::operator*() const
{
	return mStream;
}

std::fstream* nex::File::operator->()
{
	return &mStream;
}

const std::fstream* nex::File::operator->() const
{
	return &mStream;
}

void nex::File::test()
{
	size_t bytes = 8192;
	{
		File file;
		file.open("test.bin", std::ios::out | std::ios::trunc | std::ios::binary);
		*file << bytes;
	}

	{
		File file4;
		file4.open("test.bin", std::ios::in | std::ios::binary);
		bytes = 0;
		*file4 >> bytes;
	}

	std::cout << "bytes = " << bytes << std::endl;
}

void nex::StreamUtil::readString(std::istream& in, std::string& str)
{
	size_t size;
	in.read((char*)&size, sizeof(size));
	str.resize(size);
	in.read(str.data(), size);
}

void nex::StreamUtil::writeString(std::ostream& out, const std::string& str)
{
	write<size_t>(out, str.size());
	out.write(str.data(), str.size());
}