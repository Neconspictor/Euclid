#include <nex/memory/PoolAllocator.hpp>
#include <nex/util/PointerUtils.hpp>

PoolAllocator::PoolAllocator(size_t objectSize, uint64_t objectAlignment, size_t size, void* mem)
	: Allocator(size, mem), objectSize(objectSize), objectAlignment(objectAlignment)
{
	assert(objectSize >= sizeof(void*));

	//Calculate adjustment needed to keep object correctly aligned
	uint64_t adjustment = alloc::alignForwardAdjustment(mem, objectAlignment);

	freeList = (void**)nex::util::add(mem, (int)adjustment);

	size_t numObjects = (size - adjustment) / objectSize;

	void** p = freeList;

	//Initialize free blocks list
	for (size_t i = 0; i < numObjects - 1; i++)
	{
		*p = nex::util::add(p, (int)objectSize);
		p = (void**)*p;
	}

	*p = nullptr;
}

PoolAllocator::~PoolAllocator()
{
	freeList = nullptr;
}

void* PoolAllocator::alloc(size_t size, uint64_t alignment)
{
	assert(size == objectSize && alignment == objectAlignment);

	if (freeList == nullptr)
		return nullptr;

	void* p = freeList;

	freeList = (void**)(*freeList);

	usedMemory += size;
	++numAllocations;

	return p;
}

void PoolAllocator::dealloc(void* p)
{
	*((void**)p) = freeList;

	freeList = (void**)p;

	usedMemory -= objectSize;
	--numAllocations;
}