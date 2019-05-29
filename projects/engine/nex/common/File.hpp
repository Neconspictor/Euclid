#pragma once

#include <fstream>

namespace nex
{
	/**
	 * A small wrapper for std::fstream that makes buffered file processing easier.
	 */
	class File
	{
	public:
		File(size_t bufferSize = 1024);
		File(const File&) = delete;
		File& operator=(const File&) = delete;

		File(File&&) = default;
		File& operator=(File&&) = default;

		~File();

		/**
		 * Opens the file and produces a buffered stream.
		 */
		std::fstream&  open(const char* filePath, std::ios_base::openmode mode);
		std::fstream&  open(std::filesystem::path& path, std::ios_base::openmode mode);

		std::fstream& get();


		std::fstream& operator*();
		const std::fstream& operator*() const;

		std::fstream* operator->();
		const std::fstream* operator->() const;

		static void test();

	private:
		std::fstream mStream;
		std::vector<char> mBuffer;
	};

	class StreamUtil
	{
	public:
		template<typename T>
		static void readVec(std::vector<T>& vec, std::istream& in)
		{
			size_t bytes = 0;
			in >> bytes;
			vec.resize(bytes);
			in.read(vec.data(), bytes);
		}

		template<typename T>
		static void writeVec(const std::vector<T>& vec, std::ostream& out)
		{
			size_t bytes = vec.size() * sizeof(T);
			out << bytes;
			out.write(vec.data(), bytes);
		}
	};

	template<typename T>
	std::istream& operator>>(std::istream& in, std::vector<T>& vec)
	{
		size_t bytes = 0;
		in.read((char*)&bytes, sizeof(bytes));
		vec.resize(bytes / sizeof(T));
		in.read(reinterpret_cast<char*>(vec.data()), bytes);

		return in;
	}

	template<typename T>
	std::ostream& operator<<(std::ostream& out, const std::vector<T>& vec)
	{
		size_t bytes = vec.size() * sizeof(T);
		out.write((const char*)&bytes ,sizeof(bytes));
		out.write(reinterpret_cast<const char*>(vec.data()), bytes);

		return out;
	}
}