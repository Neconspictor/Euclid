#pragma once

#include <nex/memory/Allocator.hpp>

namespace nex {

	/**
	* Based on http://www.gamedev.net/page/resources/_/technical/general-programming/c-custom-memory-allocation-r3010
	*/

	class PoolAllocator : public Allocator
	{
	public:
		PoolAllocator(size_t objectSize, uint64_t objectAlignment, size_t size, void* mem);
		~PoolAllocator();

		void* alloc(size_t size, uint64_t alignment) override;

		void dealloc(void* p) override;

	private:
		PoolAllocator(const PoolAllocator&); //Prevent copies because it might cause errors
		PoolAllocator& operator=(const PoolAllocator&);

		size_t objectSize;
		uint64_t objectAlignment;

		void**     freeList;
	};
}