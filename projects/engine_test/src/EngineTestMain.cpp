#include <iostream>
#include <platform/memory/Allocator.hpp>
#include <platform/memory/LinearAllocator.hpp>
#include <platform/memory/StackAllocator.hpp>
#include <platform/memory/FreeListAllocator.hpp>
#include <platform/util/PointerUtils.hpp>
#include <platform/memory/PoolAllocator.hpp>
#include <platform/memory/ObjectPool.hpp>


using namespace std;

struct T
{
	int data[2];
};

int main(int argc, char** argv)
{

	void* memory = malloc(1024*8);
	size_t usedMemory = 0;

	// make a linear allocator and give it 1 kb memory
	LinearAllocator linear(1024, memory);
	cout << "linear size: " << linear.getSize() << endl;
	usedMemory += linear.getSize();

	StackAllocator stack(1024, platform::util::add(memory, usedMemory));
	cout << "stack size: " << stack.getSize() << endl;
	usedMemory += stack.getSize();

	FreeListAllocator freeList(1024, platform::util::add(memory, usedMemory));
	cout << "freeList size: " << freeList.getSize() << endl;
	usedMemory += freeList.getSize();

	PoolAllocator pool(sizeof(T), 4, 1024, platform::util::add(memory, usedMemory));
	cout << "pool size: " << pool.getSize() << endl;
	usedMemory += pool.getSize();

	ObjectPool<T> objectPool(platform::util::add(memory, usedMemory), 100, sizeof(T));
	cout << "object pool allocator size: " << objectPool.getAllocator()->getSize() << endl;
	usedMemory += objectPool.getAllocator()->getSize();

	cout << "used memory by allocators: " << usedMemory << endl;

	int length = 4;
	T* array = (T*)linear.alloc(sizeof(T) * length, 4);

	for (int i = 0; i < length; ++i)
	{
		array[i].data[0] = i;
		array[i].data[1] = i*2;
	}

	for (int i = 0; i < length; ++i)
	{
		cout << "array[" << i << "] = (" << array[i].data[0] << ", " << array[i].data[1] << ")" << endl;
	}

	cout << "linear allocated bytes: " << linear.getUsedMemory() << endl;

	T* stackElem = (T*)stack.alloc(sizeof(T), 4);

	T* arrayStack = (T*)stack.alloc(sizeof(T) * length, 4);

	for (int i = 0; i < length; ++i)
	{
		arrayStack[i].data[0] = i;
		arrayStack[i].data[1] = i * 2;
	}

	for (int i = 0; i < length; ++i)
	{
		cout << "array[" << i << "] = (" << arrayStack[i].data[0] << ", " << arrayStack[i].data[1] << ")" << endl;
	}

	cout << "stack allocated bytes: " << stack.getUsedMemory() << endl;

	stack.dealloc(arrayStack);
	cout << "stack allocated bytes: " << stack.getUsedMemory() << endl;
	stack.dealloc(stackElem);
	cout << "stack allocated bytes: " << stack.getUsedMemory() << endl;


	T* arrayFreeList = (T*)freeList.alloc(sizeof(T) * length, 8);

	T* arrayFreeList2 = (T*)freeList.alloc(sizeof(T) * 100, 8);

	T* arrayFreeList3 = (T*)freeList.alloc(sizeof(T) * 16, 8);

	cout << "sizeof(T) = " << sizeof(T) << endl;

	for (int i = 0; i < length; ++i)
	{
		arrayFreeList[i].data[0] = i;
		arrayFreeList[i].data[1] = i * 2;
	}

	for (int i = 0; i < length; ++i)
	{
		cout << "arrayFreeList[" << i << "] = (" << arrayFreeList[i].data[0] << ", " << arrayFreeList[i].data[1] << ")" << endl;
	}

	cout << "free list used memory = " << freeList.getUsedMemory() << endl;

	freeList.dealloc(arrayFreeList3);
	freeList.dealloc(arrayFreeList);
	freeList.dealloc(arrayFreeList2);
	cout << "free list used memory = " << freeList.getUsedMemory() << endl;

	T* poolElem = (T*)pool.alloc(sizeof(T), 4);
	T* poolElem2 = (T*)pool.alloc(sizeof(T), 4);
	cout << "pool used memory = " << pool.getUsedMemory() << endl;

	pool.dealloc(poolElem);
	cout << "pool used memory = " << pool.getUsedMemory() << endl;
	poolElem = (T*)pool.alloc(sizeof(T), 4);
	pool.dealloc(poolElem2);
	cout << "pool used memory = " << pool.getUsedMemory() << endl;
	pool.dealloc(poolElem);
	cout << "pool used memory = " << pool.getUsedMemory() << endl;


	cout << "object pool capacity = " << objectPool.capacity() << endl;
	cout << "object pool number of created objects = " << objectPool.getNumAllocations() << endl;

	T* object = objectPool.alloc();
	cout << "object pool number of created objects = " << objectPool.getNumAllocations() << endl;
	cout << "object pool free slots = " << objectPool.size() << endl;

	objectPool.dealloc(object);
	cout << "object pool free slots = " << objectPool.size() << endl;

	// free the memory
	linear.clear();
	free(memory);
	return EXIT_SUCCESS;
}