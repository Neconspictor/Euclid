#include <platform/memory/LinearAllocator.hpp>
#include <iostream>

LinearAllocator::LinearAllocator(size_t size, void* start)
	: Allocator(size, start), currentPos(start)
{
	assert(size > 0);
}

LinearAllocator::~LinearAllocator()
{
	currentPos = nullptr;
}

void* LinearAllocator::alloc(size_t size, uint64_t alignment)
{
	assert(size != 0);

	uint64_t adjustment = alloc::alignForwardAdjustment(currentPos, alignment);
	std::cout << "adjustment = " << adjustment << std::endl;

	if (usedMemory + adjustment + size > this->size)
		return nullptr;

	uint64_t* alignedAddress = (uint64_t*)currentPos + adjustment;

	currentPos = (void*)(alignedAddress + size);

	usedMemory += size + adjustment;
	++numAllocations;

	return (void*)alignedAddress;
}

void LinearAllocator::dealloc(void* p)
{
	assert(false && "Use clear() instead");
}

void LinearAllocator::clear()
{
	numAllocations = 0;
	usedMemory = 0;

	currentPos = start;
}