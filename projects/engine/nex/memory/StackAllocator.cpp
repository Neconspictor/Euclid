#include <nex/memory/StackAllocator.hpp>
#include <nex/util/PointerUtils.hpp>
#include <iostream>

StackAllocator::StackAllocator(size_t size, void* start)
	: Allocator(size, start), currentPos(start)
{
	assert(size > 0);

#if _DEBUG
	prevPosition = nullptr;
#endif
}

StackAllocator::~StackAllocator()
{
#if _DEBUG
	prevPosition = nullptr;
#endif

	currentPos = nullptr;
}

void* StackAllocator::alloc(size_t size, uint64_t alignment)
{
	assert(size != 0);

	uint64_t adjustment = alloc::alignForwardAdjustmentWithHeader(currentPos, alignment, sizeof(AllocationHeader));

	std::cout << "header size = " << sizeof(AllocationHeader) << std::endl;
	std::cout << "adjustment = " << adjustment << std::endl;

	if (usedMemory + adjustment + size > this->size)
		return nullptr;

	void* aligned_address = nex::util::add(currentPos, (int)adjustment);

	//Add Allocation Header
	AllocationHeader* header = (AllocationHeader*)(nex::util::subtract(aligned_address, sizeof(AllocationHeader)));

	header->adjustment = adjustment;

#if _DEBUG
	header->prev_address = prevPosition;

	prevPosition = aligned_address;
#endif

	currentPos = nex::util::add(aligned_address, (int)size);

	usedMemory += size + adjustment;
	++numAllocations;

	return aligned_address;
}

void StackAllocator::dealloc(void* p)
{
	//assert(p == prevPosition);

	//Access the AllocationHeader in the bytes before p
	AllocationHeader* header = (AllocationHeader*)(nex::util::subtract(p, sizeof(AllocationHeader)));

	usedMemory -= (size_t)currentPos - (size_t)p + header->adjustment;

	currentPos = nex::util::subtract(p, (int)header->adjustment);

#if _DEBUG
	prevPosition = header->prev_address;
#endif

	--numAllocations;
}