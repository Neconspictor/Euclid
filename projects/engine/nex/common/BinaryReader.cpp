#include "BinaryReader.h"

#include <iostream>  
#include <fstream>  


// Constructor reads from the filesystem.
BinaryReader::BinaryReader(char const* fileName) :
	mBegin(nullptr),
    mPos(nullptr),
    mEnd(nullptr)
{
	using namespace std;
    size_t dataSize;
    try
    {
		ReadEntireFile(fileName, mOwnedData, &dataSize);
		mBegin = mOwnedData.get();
		mPos = mOwnedData.get();
		mEnd = mOwnedData.get() + dataSize;

    } catch (std::runtime_error& e)
    {
		throw std::runtime_error("Couldn't read file '" + string(fileName) + "' - cause: " + string(e.what()));
    }
}


// Constructor reads from an existing memory buffer.
BinaryReader::BinaryReader(char const* dataBlob, size_t dataSize) :
	mBegin(dataBlob),
    mPos(dataBlob),
    mEnd(dataBlob + dataSize)
{
}


size_t BinaryReader::getTotalSize() const
{
	return mEnd - mBegin;
}

size_t BinaryReader::getAmountLeftBytes() const
{
	return mEnd - mPos;
}

// Reads from the filesystem into memory.
void BinaryReader::ReadEntireFile(char const* fileName, std::unique_ptr<char[]>& data, size_t* dataSize)
{
	using namespace std;

	ifstream file;
	file.exceptions(ifstream::badbit < ifstream::failbit);

	file.open(fileName, ios::binary);

	streampos fileSize = getFileSize(file);

    // Create enough space for the file data.
	char* memory = nullptr;
	try
	{
		memory = new char[fileSize];
		
	} catch(const bad_alloc&)
	{
		throw std::runtime_error("BinaryReader::ReadEntireFile(): Couldn't allocate memory!");
	}

    data.reset(memory);

	file.read((char*)data.get(), fileSize);
    *dataSize = fileSize;
}

std::streampos BinaryReader::getFileSize(std::ifstream& file)
{
	using namespace std;

	streampos backup = file.tellg();
	file.seekg(0, ios::beg);

	streampos begin = file.tellg();
	
	file.seekg(0, ios::end);
	streampos fsize = file.tellg() - begin;

	// restore state 
	file.seekg(backup, ios_base::beg);

	return fsize;
}