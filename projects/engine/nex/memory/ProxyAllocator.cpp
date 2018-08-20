
#include <nex/memory/ProxyAllocator.hpp>

ProxyAllocator::ProxyAllocator(Allocator& allocator)
	: Allocator(allocator.getSize(), allocator.getStart()), _allocator(allocator)
{}

ProxyAllocator::~ProxyAllocator()
{}

void* ProxyAllocator::alloc(size_t size, uint64_t alignment)
{
	assert(size != 0);
	++numAllocations;

	size_t mem = _allocator.getUsedMemory();

	void* p = _allocator.alloc(size, alignment);

	usedMemory += _allocator.getUsedMemory() - mem;

	return p;
}

void ProxyAllocator::dealloc(void* p)
{
	--numAllocations;

	size_t mem = _allocator.getUsedMemory();

	_allocator.dealloc(p);

	usedMemory -= mem - _allocator.getUsedMemory();
}