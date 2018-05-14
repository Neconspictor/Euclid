#pragma once

/**
* Based on http://www.gamedev.net/page/resources/_/technical/general-programming/c-custom-memory-allocation-r3010
*/
#include <platform/memory/Allocator.hpp>


class FreeListAllocator : public Allocator
{
public:
	FreeListAllocator(size_t size, void* start);
	~FreeListAllocator();

	void* alloc(size_t size, uint64_t alignment) override;

	void dealloc(void* p) override;

private:

	struct AllocationHeader
	{
		size_t size;
		uint64_t adjustment;
	};

	struct FreeBlock
	{
		size_t     size;
		FreeBlock* next;
	};

	FreeListAllocator(const FreeListAllocator&); //Prevent copies because it might cause errors
	FreeListAllocator& operator=(const FreeListAllocator&);

	FreeBlock* _free_blocks;
};