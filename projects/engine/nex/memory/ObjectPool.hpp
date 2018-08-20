#pragma once
#include <nex/memory/PoolAllocator.hpp>

template<class T> 
class ObjectPool
{
public:
	explicit ObjectPool(void* memoryStart, size_t objectNumber, size_t alignment)
		: allocator(sizeof(T), alignment, sizeof(T) * objectNumber, memoryStart),
		alignment(alignment), mCapacity(objectNumber)
	{}

	virtual ~ObjectPool() {}

	size_t size() const
	{
		return mCapacity - allocator.getNumAllocs();
	}

	T* alloc()
	{
		return reinterpret_cast<T*>(allocator.alloc(sizeof(T), alignment));
	}

	size_t capacity() const
	{
		return mCapacity;
	}


	PoolAllocator* getAllocator()
	{
		return &allocator;
	}

	size_t getNumAllocations() const
	{
		return allocator.getNumAllocs();
	}

	void dealloc(T* object)
	{
		allocator.dealloc(object);
	}

protected:
	PoolAllocator allocator;
	size_t alignment;
	size_t mCapacity;
};