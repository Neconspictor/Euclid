#pragma once

#include <cstdlib>

/**
 * A memory manager allocates on construction a contiguous memory block. Clients can request
 * a contiguous memory block of any size (up to the size the manager allocated initially).
 */
class MemoryManager
{
public:
	/**
	 * Constructs a new memory manager. Tries to allocate 
	 * a contiguous memory block of 'byteSize'. 
	 * An escption is thrown, if the memory couldn't be allocated.
	 */
	MemoryManager(size_t byteSize);

	virtual ~MemoryManager();

	/**
	 * Provides the total amount of bytes, that can be allocated 
	 * by other objects.
	 */
	size_t capacity() const;

	/**
	 * Provides the amount of memory the memory manager can still provide.
	 */
	size_t freeSpace() const;

	/**
	 * Provides a contiguous memory block. If the memory manager hasn't enough free memory space,
	 * an exception is thrown.
	 */
	void* getMemory(size_t byteCount);

	/**
	 * Provides the amount of bytes that are allocated.
	 */
	size_t size() const;

	/**
	 * Releases all allocated memory. No now allocations 
	 * can be done anymore.
	 */
	void shutdown();

protected:
	void* memory;
	size_t mSize;
	bool mShutdown;
};