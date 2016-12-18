#include <platform/memory/MemoryManager.hpp>
#include <cstdlib>
#include <platform/util/PointerUtils.hpp>
#include <cassert>

MemoryManager::MemoryManager(size_t byteSize)
{
	memory = malloc(byteSize);
	mShutdown = false;
	mSize = 0;
}

MemoryManager::~MemoryManager()
{
}

size_t MemoryManager::capacity() const
{
	return sizeof(memory);
}

size_t MemoryManager::freeSpace() const
{
	return sizeof(memory) - mSize;
}

void* MemoryManager::getMemory(size_t byteCount)
{
	assert(!mShutdown);
	void* result = platform::util::add(memory, mSize);
	mSize += byteCount;
	return result;
}

size_t MemoryManager::size() const
{
	return mSize;
}

void MemoryManager::shutdown()
{
	free(memory);
	mShutdown = true;
}