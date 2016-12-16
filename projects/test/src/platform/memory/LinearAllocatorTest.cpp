#include <gtest/gtest.h>
#include <platform/memory/LinearAllocator.hpp>

struct T
{
	int data[2];
};

TEST(linear_allocator, startup)
{
	char memory[512];
	LinearAllocator linear(512, &memory);

	ASSERT_TRUE(linear.getSize() == 512);
}

TEST(linear_allocator, alloc_array)
{
	
}

// The fixture for testing class Foo.
class LinearAllocatorTest : public ::testing::Test {
protected:
	// You can remove any or all of the following functions if its body
	// is empty.

	LinearAllocatorTest() {
		memory = malloc(memorySize);
		// You can do set-up work for each test here.
		allocator = LinearAllocator(memorySize, memory);
	}

	virtual ~LinearAllocatorTest() {
		// You can do clean-up work that doesn't throw exceptions here.
		free(memory);
	}

	// If the constructor and destructor are not enough for setting up
	// and cleaning up each test, you can define the following methods:

	virtual void SetUp() {
		// Code here will be called immediately after the constructor (right
		// before each test).
	}

	virtual void TearDown() {
		// Code here will be called immediately after each test (right
		// before the destructor).
	}

	// Objects declared here can be used by all tests in the test case for Foo.

public:
	LinearAllocator allocator;
	void* memory;
	static const size_t memorySize = 512;
};

// Tests that the Foo::Bar() method does Abc.
TEST_F(LinearAllocatorTest, usedMemory) {
	ASSERT_TRUE(allocator.getSize() == memorySize);
}

// Tests that Foo does Xyz.
TEST_F(LinearAllocatorTest, allocation) {
	int length = 4;
	T* array = (T*)allocator.alloc(sizeof(T) * length, sizeof(T));

	ASSERT_TRUE(allocator.getUsedMemory() == sizeof(T) * length);

	allocator.clear();
	ASSERT_TRUE(allocator.getUsedMemory() == 0);
}