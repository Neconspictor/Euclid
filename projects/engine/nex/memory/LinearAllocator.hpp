#pragma once
/**
 * Based on http://www.gamedev.net/page/resources/_/technical/general-programming/c-custom-memory-allocation-r3010
 */
#include <nex/memory/Allocator.hpp>

/**
 * A linear allocator makes allocations by simple moving its cursor forward to the first free address.
 * Additionally a linear allocator doesn't provide the ability to deallocate individual allocations.
 * Instead it provides a clear function to deallocate all allocated memory at once.
 *
 * Advantage: The disadvantage of not allowing to deallocate individual allocations enables the allocator 
 * to allocate memory with nearly no memory and performance overhead.
 */
class LinearAllocator : public Allocator
{
public:
	LinearAllocator();
	LinearAllocator(size_t size, void* start);
	LinearAllocator& operator=(LinearAllocator&& other);
	~LinearAllocator();


	void* alloc(size_t size, uint64_t alignment) override;

	/**
	 * This function shouldn't be used for a linear allocator! Use LinearAllocator::clear() instead!
	 */
	void dealloc(void* p) override;

	/**
	 * Dealloctes all allocated memory.
	 */
	void clear();

private:
	LinearAllocator(const LinearAllocator&); //Prevent copies because it might cause errors
	LinearAllocator& operator=(const LinearAllocator&);

	void* currentPos;
};