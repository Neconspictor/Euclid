#pragma once
#include <cassert>
#include <cstdint>

/**
* Based on http://www.gamedev.net/page/resources/_/technical/general-programming/c-custom-memory-allocation-r3010
* This class functions as a base class for custom memory allocators.
*/
class Allocator
{
public:

	/**
	 * Creates a new allocator, that is able to manage to alloc 'size' bytes.
	 * The 'memoryStart' pointer is a pointer to the start address of memory
	 * that the allocator can use.
	 * NOTE: It is important that the memory address range [memoryStart, (char*)memoryStart + size]
	 * Is valid for the lifetime of the allocator.
	 */
	Allocator (size_t size, void* memoryStart) {
		start = memoryStart;
		this->size = size;

		usedMemory = 0;
		numAllocations = 0;
	}

	Allocator& operator=(Allocator&& other) {
		if (this == &other) return *this;
		start = other.start;
		size = other.size;
		usedMemory = other.usedMemory;
		numAllocations = other.numAllocations;

		other.start = nullptr;
		other.size = 0;
		other.usedMemory = 0;
		other.numAllocations = 0;

		return *this;
	}

	/**
	 * NOTE: numAllocations and usedMemory have to be zero.
	 */
	virtual ~Allocator()
	{
		assert(numAllocations == 0 && usedMemory == 0);

		start = nullptr;
		size = 0;
	}

	/**
	 * Reserves memory using a specified alignment and returns a pointer to the beginning of the memory.
	 */
	virtual void* alloc(size_t size, uint64_t alignment = 4) = 0;

	/**
	 * Frees memory, that was previously allocated by the alloc(size_t, uint32_t) function.
	 */
	virtual void dealloc(void* memory) = 0;

	/**
	 * Returns a pointer to the start of memory the allocator manages.
	 */
	void* getStart() const
	{
		return start;
	}

	/**
	 * Returns the amount of bytes the allocator is able to allocate.
	 */
	size_t getSize()  const
	{
		return size;
	}

	/**
	 * Provides the amount of memory that is allocated by this allocator.
	 */
	size_t getUsedMemory() const
	{
		return usedMemory;
	}

	/**
	 * Provides the number of allocations done by the allocator.
	 */
	size_t getNumAllocs() const
	{
		return numAllocations;
	}

protected:
	void* start;
	size_t size;

	size_t usedMemory;
	size_t numAllocations;
};

namespace alloc
{
	template <class T> T* alloc(Allocator& allocator)
	{
		// the placement new operator doesn't alloc any memory! 
		return new (allocator.alloc(sizeof(T), __alignof(T))) T;
	}

	/**
	 * Replacement function of the new operator.
	 */
	template <class T> T* alloc(Allocator& allocator, const T& t)
	{
		return new (allocator.alloc(sizeof(T), __alignof(T))) T(t);
	}

	/**
	 * Replacement function of the delete operator for a custom allocator.
	 */
	template<class T> void dealloc(Allocator& allocator, T& object)
	{
		object.~T();
		allocator.dealloc(&object);
	}

	/**
	 * Allocates an array of objects of type 'T'´ using a given allocator. 
	 * Additionally to the array a header with a byte size of (TODO) is allocated.
	 */
	template<class T> T* allocArray(Allocator& allocator, size_t length)
	{
		assert(length != 0);

		size_t headerSize = sizeof(size_t) / sizeof(T);
		if (sizeof(size_t) % sizeof(T) > 0)
			headerSize += 1;

		//Allocate an etxra (or more) T to store array length in the bytes before the (aligned) array.
		T* memory = ((T*)allocator.alloc(sizeof(T)*(length + headerSize), __alignof(T))) + headerSize;

		*(((size_t*)memory) - 1) = length;

		for (size_t i = 0; i < length; i++)
			new (&memory[i]) T;

		return memory;
	}

	/**
	 * Deallocates an array of objects having type 'T'. For deallocation a given allocator is used. 
	 * Note: It is assumed, that the array of objects was previously allocated by using the function
	 * allocArray(Allocator&, size_t) called with the same allocator specified here for deallocation.
	 */
	template<class T> void deallocArray(Allocator& allocator, T* array)
	{
		assert(array != nullptr);

		size_t length = *(((size_t*)array) - 1);

		for (size_t i = 0; i < length; i++)
			array[i].~T();

		//Calculate how much extra memory was allocated to store the length before the array
		size_t headerSize = sizeof(size_t) / sizeof(T);

		if (sizeof(size_t) % sizeof(T) > 0)
			headerSize += 1;

		allocator.dealloc(array - headerSize);
	}

	/**
	* Aligns the given address pointer to a number of bytes.
	* Some General information about memory address alignment:
	* Alignment means to divide the whole memory space into pieces of a specific byte count (the alignment number).
	* To align a address pointer forwardly, means to move the address pointer forward (never backwards!), 
	* so that it points to the beginning of one of these memory pieces.
	* If the pointer points already to the beginning of one of these pieces, the pointer is already aligned and no shifting
	* has to be done.
	* @param address: The address to align.
	* @param alignment: The byte alignment number. Must be a power of 2
	* @return: The aligned memory address.
	*/
	inline void* alignForward(void* address, uint64_t alignment)
	{
		// x_aligned = (x + (alignment -1)) & ~(alignment -1) 
		return (void*)((reinterpret_cast<int64_t>(address) + static_cast<int64_t>(alignment - 1)) & 
															 static_cast<int64_t>(~(alignment - 1)));
	}

	/**
	 * Provides the number of bytes the given address would be shifted by an alignment. 
	 * @param address: The address to calculate the shifting bytes from.
	 * @param alignment: The byte alignment number. Must be a power of 2
	 */
	inline uint64_t alignForwardAdjustment(const void* address, uint64_t alignment)
	{
		uint64_t adjustment = alignment - (reinterpret_cast<int64_t>(address) & static_cast<int64_t>(alignment - 1));

		if (adjustment == alignment)
			return 0; //already aligned

		return adjustment;
	}


	/** 
	 * Calculates the number of bytes to align a given address pointer to a specific alignment number. The function works
	 * similar to alignForwardAdjustment(const void*, uint64_t), but takes also a header size parameter. With that
	 * one can specifiy a minimum amount of bytes, the pointer would be shifted. This way, one gets an aligned pointer, but
	 * between the original address pointer and the shifted one lies enough memory to place a header of size 'headerSize'.
	 * 
	 * Some allocators need a header on the beginning of the allocated memory. To reduce space overhead one can place
	 * the header in the alignment space. Therefore this function is intended to be used.
	 */
	inline uint64_t alignForwardAdjustmentWithHeader(const void* address, uint64_t alignment, uint64_t headerSize)
	{
		uint64_t adjustment = alignForwardAdjustment(address, alignment);

		uint64_t neededSpace = headerSize;

		if (adjustment < neededSpace)
		{
			neededSpace -= adjustment;

			//Increase adjustment to fit header
			adjustment += alignment * (neededSpace / alignment);

			if (neededSpace % alignment > 0)
				adjustment += alignment;
		}

		return adjustment;
	}
}
