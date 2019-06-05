#pragma once

#include <fstream>
#include <string>

namespace nex
{
	class BinStream : public std::fstream
	{
	public:
		BinStream(size_t bufferSize = 1024);
		BinStream(const BinStream&) = delete;
		BinStream& operator=(const BinStream&) = delete;

		BinStream(BinStream&&) = delete;
		BinStream& operator=(BinStream&&) = delete;

		virtual ~BinStream() noexcept;

		void open(const char* filePath, std::ios_base::openmode mode);
		void open(const std::filesystem::path& path, std::ios_base::openmode mode);

		static void test();

	private:
		std::vector<char> mBuffer;
		std::filesystem::path mFile;
	};


	template<typename T>
	nex::BinStream& operator>>(nex::BinStream& in, T& item)
	{
		in.read((char*)&item, sizeof(T));
		return in;
	}

	template<typename T>
	nex::BinStream& operator<<(nex::BinStream& out, const T& item)
	{
		out.write((const char*)&item, sizeof(T));
		return out;
	}

	nex::BinStream& operator>>(nex::BinStream& in, std::string& str);

	nex::BinStream& operator<<(nex::BinStream& out, const std::string& str);


	template<typename T>
	nex::BinStream& operator>>(nex::BinStream& in, std::vector<T>& vec)
	{
		size_t count = 0;
		in >> count;
		
		if (count > 0)
			vec.resize(count);

		for (size_t i = 0; i < count; ++i)
		{
			in >> vec[i];
		}

		return in;
	}

	template<typename T>
	nex::BinStream& operator<<(nex::BinStream& out, const std::vector<T>& vec)
	{
		const size_t count = vec.size();
		out << count;
		for (const auto& item : vec)
		{
			out << item;
		}

		return out;
	}

	/**
	 * Specialization for char vectors (improved performance)
	 */
	nex::BinStream& operator>>(nex::BinStream& in, std::vector<char>& vec);

	/**
	 * Specialization for char vectors (improved performance)
	 */
	nex::BinStream& operator<<(nex::BinStream& out, const std::vector<char>& vec);
}