#include <numeric>

#include <gtest/gtest.h>

#include <Maia/Utilities/Containers/Pools/MemoryPool.hpp>

using namespace Maia::Utilities;

namespace Maia::Utilities::Test
{
	class Memory_pool_test : public ::testing::Test
	{
	};

	TEST_F(Memory_pool_test, DefaultConstructorCreatesEmptyMemoryPool)
	{
		Memory_pool<int> memory_pool;
		EXPECT_TRUE(memory_pool.empty());
	}
	TEST_F(Memory_pool_test, CustomConstructorCreatesEmptyMemoryPool)
	{
		Memory_pool<int> memory_pool(1);
		EXPECT_TRUE(memory_pool.empty());
	}

	TEST_F(Memory_pool_test, DefaultConstructorCreatesMemoryPoolWithSize0)
	{
		Memory_pool<int> memory_pool;
		EXPECT_EQ(0, memory_pool.size());
	}
	TEST_F(Memory_pool_test, CustomConstructorCreatesMemoryPoolWithSize0)
	{
		Memory_pool<int> memory_pool(1);
		EXPECT_EQ(0, memory_pool.size());
	}

	TEST_F(Memory_pool_test, MaxSizeReturnsNonZero)
	{
		Memory_pool<int> memoryPool;
		EXPECT_NE(0, memoryPool.max_size());
	}

	TEST_F(Memory_pool_test, ReserveAffectsCapacity)
	{
		Memory_pool<int> memoryPool;
		EXPECT_EQ(0, memoryPool.capacity());

		memoryPool.reserve(1);
		EXPECT_EQ(1, memoryPool.capacity());
	}

	TEST_F(Memory_pool_test, ClearAffectsSize)
	{
		Memory_pool<int> memory_pool(1);

		memory_pool.emplace(0);
		memory_pool.clear();

		EXPECT_EQ(0, memory_pool.size());
	}

	TEST_F(Memory_pool_test, EmplaceReturnsIterator)
	{
		Memory_pool<int> memory_pool(2);

		ASSERT_NO_THROW(memory_pool.emplace(0));

		auto position = memory_pool.emplace(1);
		ASSERT_NO_THROW(*position);

		const auto& reference = *position;
		EXPECT_EQ(reference, 1);
	}
	TEST_F(Memory_pool_test, EmplaceAffectsSize)
	{
		Memory_pool<int> memory_pool(1);
		ASSERT_TRUE(memory_pool.empty());

		ASSERT_NO_THROW(memory_pool.emplace(0));
		ASSERT_FALSE(memory_pool.empty());
	}
	TEST_F(Memory_pool_test, EmplaceThrowsExceptionIfNoCapacity)
	{
		Memory_pool<int> memoryPool;
		EXPECT_THROW(memoryPool.emplace(0), std::out_of_range);
	}

	TEST_F(Memory_pool_test, EraseElementAffectsSize)
	{
		Memory_pool<int> memory_pool(1);

		auto position = memory_pool.emplace(2);
		ASSERT_FALSE(memory_pool.empty());

		EXPECT_NO_THROW(memory_pool.erase(position));
		EXPECT_TRUE(memory_pool.empty());
	}

	TEST_F(Memory_pool_test, Swap)
	{
		Memory_pool<int> memory_pool_0(1);
		memory_pool_0.emplace(1);

		Memory_pool<int> memory_pool_1(1);
		memory_pool_1.emplace(2);

		ASSERT_NO_THROW(memory_pool_0.swap(memory_pool_1));

		EXPECT_EQ(*memory_pool_0.begin(), 2);
		EXPECT_EQ(*memory_pool_1.begin(), 1);
	}
}
