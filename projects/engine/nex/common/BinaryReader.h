#pragma once

#include <memory>
#include <exception>
#include <stdexcept>
#include <type_traits>
#include <fstream>

namespace nex
{
	// Helper for reading binary data, either from the filesystem a memory buffer.
	class BinaryReader
	{
	public:
		explicit BinaryReader(char const* fileName);
		BinaryReader(char const* dataBlob, size_t dataSize);

		BinaryReader(BinaryReader const&) = delete;
		BinaryReader& operator= (BinaryReader const&) = delete;

		// Reads a single value.
		template<typename T> T const& Read()
		{
			return *ReadArray<T>(1);
		}


		// Reads an array of values.
		template<typename T> T const* ReadArray(size_t elementCount)
		{
			static_assert(std::is_pod<T>::value, "Can only read plain-old-data types");

			char const* newPos = mPos + sizeof(T) * elementCount;

			if (newPos < mPos)
				throw std::overflow_error("ReadArray");

			if (newPos > mEnd)
				throw std::exception("End of file");

			auto result = reinterpret_cast<T const*>(mPos);

			mPos = newPos;

			return result;
		}

		size_t getTotalSize() const;

		size_t getAmountLeftBytes() const;


		// Lower level helper reads directly from the filesystem into memory.
		static void ReadEntireFile(char const* fileName, std::unique_ptr<char[]>& data, size_t* dataSize);


	private:

		static std::streampos getFileSize(std::ifstream& file);


		char const* mBegin;

		// The data currently being read.
		char const* mPos;
		char const* mEnd;

		std::unique_ptr<char[]> mOwnedData;
	};
}