#pragma once

/**
* Based on http://www.gamedev.net/page/resources/_/technical/general-programming/c-custom-memory-allocation-r3010
*/
#include <platform/memory/Allocator.hpp>

class ProxyAllocator : public Allocator
{
public:
	ProxyAllocator(Allocator& allocator);
	~ProxyAllocator();

	void* alloc(size_t size, uint64_t alignment) override;

	void dealloc(void* p) override;

private:
	ProxyAllocator(const ProxyAllocator&); //Prevent copies because it might cause errors
	ProxyAllocator& operator=(const ProxyAllocator&);

	Allocator& _allocator;
};