#pragma once

/**
 * Based on http://www.gamedev.net/page/resources/_/technical/general-programming/c-custom-memory-allocation-r3010
 */
#include <nex/memory/Allocator.hpp>

namespace nex {

	class StackAllocator : public Allocator
	{
	public:
		StackAllocator(size_t size, void* start);
		~StackAllocator();

		void* alloc(size_t size, uint64_t alignment) override;

		void dealloc(void* p) override;

	private:
		StackAllocator(const StackAllocator&); //Prevent copies because it might cause errors
		StackAllocator& operator=(const StackAllocator&);

		struct AllocationHeader
		{
#if _DEBUG
			void* prev_address;
#endif
			uint64_t adjustment;
		};

#if _DEBUG
		void* prevPosition;
#endif

		void*  currentPos;
	};
}
