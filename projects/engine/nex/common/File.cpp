#include <nex/common/File.hpp>
#include "nex/util/ExceptionHandling.hpp"
#include "Log.hpp"

nex::BinStream::BinStream(size_t bufferSize) : std::fstream(), mBuffer(bufferSize)
{
	rdbuf()->pubsetbuf(mBuffer.data(), mBuffer.size());
	exceptions(std::ofstream::failbit | std::ofstream::badbit);
}

nex::BinStream::~BinStream() noexcept = default;

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

nex::BinStream& nex::operator>>(nex::BinStream& in, std::string& str)
{
	size_t bytes = 0;
	in >> bytes;
	str.resize(bytes);
	in.read(str.data(), bytes);
	return in;
}

nex::BinStream& nex::operator<<(nex::BinStream& out, const std::string& str)
{
	out << str.size();
	out.write(str.data(), str.size());
	return out;
}

nex::BinStream& nex::operator>>(nex::BinStream& in, std::vector<char>& vec)
{
	size_t bytes = 0;
	in >> bytes;
	const size_t count = bytes;
	vec.resize(count);
	in.read(vec.data(), bytes);

	return in;
}

nex::BinStream& nex::operator<<(nex::BinStream& out, const std::vector<char>& vec)
{
	const size_t bytes = vec.size();
	out << bytes;
	out.write(vec.data(), bytes);

	return out;
}
