#pragma once

/**
	 * This class represents an array of data. The memory won't be initialized and no destructors are called when freeing memory in
	 * the class's destructor. Thus this class cannot be used for classes/structs that contain not simple or array-like data
	 * (e.g. dynamic allocated memory).
	 */
template<class Type>
class PerformanceCBuffer {
public:

	explicit PerformanceCBuffer() : mSize(0), mMemory(nullptr){
	}

	PerformanceCBuffer(size_t size) : mSize(size) {
		mMemory = new char[size * sizeof(Type)];
	}

	~PerformanceCBuffer() {

		delete[] mMemory;
	}

	/**
	 * Note: The memory might not be initialized!
	 */
	Type& operator[] (size_t n) {
		return (Type&)&mMemory[sizeof(Type) * n];
	}

	/**
	 * Note: The memory might not be initialized!
	 */
	const Type& operator[] (size_t n) const {
		return (const Type&)&mMemory[sizeof(Type) * n];
	}

	/**
	 * Note: The memory might not be initialized!
	 */
	Type* data() {
		return (Type*)mMemory;
	}

	/**
	 * Note: The memory might not be initialized!
	 */
	const Type* data() const {
		return (const Type*)mMemory;
	}

	/**
	 * Resizes the array if size != current size. Any content gets lost. 
	 * data() will provide uninitialized memory.
	 */
	void resize(size_t size) {
		// Nothing to do?
		if (size == mSize) return;

		delete[] mMemory;
		mMemory = new char[size * sizeof(Type)];
		mSize = size;
	}

	size_t size() const {
		return mSize;
	}

	size_t memSize() const {
		return mSize * sizeof(Type);
	}


private:
	char* mMemory;
	size_t mSize;
};