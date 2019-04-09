#include <numeric>
#include <vector>

#include <gtest/gtest.h>

#include <Maia/Utilities/Containers/Pools/ContiguousMemoryPool.hpp>

using namespace Maia::Utilities;

namespace Maia::Utilities::Test
{
	class Contiguous_memory_pool_test : public ::testing::Test
	{
	};

	TEST_F(Contiguous_memory_pool_test, DefaultConstructorCreatesEmptyMemoryPool)
	{
		Contiguous_memory_pool<int> memoryPool;
		EXPECT_TRUE(memoryPool.empty());
	}
	TEST_F(Contiguous_memory_pool_test, CustomConstructorCreatesEmptyMemoryPool)
	{
		Contiguous_memory_pool<int> memoryPool(1);
		EXPECT_TRUE(memoryPool.empty());
	}

	TEST_F(Contiguous_memory_pool_test, DefaultConstructorCreatesMemoryPoolWithSize0)
	{
		Contiguous_memory_pool<int> memoryPool;
		EXPECT_EQ(0, memoryPool.size());
	}
	TEST_F(Contiguous_memory_pool_test, CustomConstructorCreatesMemoryPoolWithSize0)
	{
		Contiguous_memory_pool<int> memoryPool(1);
		EXPECT_EQ(0, memoryPool.size());
	}

	TEST_F(Contiguous_memory_pool_test, MaxSizeReturnsNonZero)
	{
		Contiguous_memory_pool<int> memoryPool;
		EXPECT_NE(0, memoryPool.max_size());
	}

	TEST_F(Contiguous_memory_pool_test, ReserveAffectsCapacity)
	{
		Contiguous_memory_pool<int> memoryPool;
		EXPECT_EQ(0, memoryPool.capacity());

		memoryPool.reserve(1);
		EXPECT_EQ(1, memoryPool.capacity());
	}

	TEST_F(Contiguous_memory_pool_test, ClearAffectsSize)
	{
		Contiguous_memory_pool<int> memoryPool(1);

		memoryPool.emplace_back(0);
		memoryPool.clear();

		EXPECT_EQ(0, memoryPool.size());
	}

	TEST_F(Contiguous_memory_pool_test, EmplaceBackReturnsReference)
	{
		Contiguous_memory_pool<int> memoryPool(2);

		ASSERT_NO_THROW(memoryPool.emplace_back(0));

		auto& reference = memoryPool.emplace_back(1);
		EXPECT_EQ(reference, 1);
	}
	TEST_F(Contiguous_memory_pool_test, EmplaceBackAffectsSize)
	{
		Contiguous_memory_pool<int> memoryPool(1);
		ASSERT_TRUE(memoryPool.empty());

		ASSERT_NO_THROW(memoryPool.emplace_back(0));
		ASSERT_FALSE(memoryPool.empty());
	}
	TEST_F(Contiguous_memory_pool_test, EmplaceBackThrowsExceptionIfSizeEqualsCapacity)
	{
		Contiguous_memory_pool<int> memoryPool;
		ASSERT_EQ(memoryPool.size(), memoryPool.capacity());
		EXPECT_THROW(memoryPool.emplace_back(0), std::out_of_range);
	}

	TEST_F(Contiguous_memory_pool_test, swap_with_back_and_pop_back)
	{
		Contiguous_memory_pool<int> memoryPool(3);

		memoryPool.emplace_back(0);
		memoryPool.emplace_back(1);
		memoryPool.emplace_back(2);
		ASSERT_EQ(3, memoryPool.size());
		ASSERT_EQ(0, *memoryPool.begin());

		memoryPool.swap_with_back_and_pop_back(memoryPool.begin());
		EXPECT_EQ(2, memoryPool.size());
		EXPECT_EQ(2, *memoryPool.begin());

		memoryPool.swap_with_back_and_pop_back(memoryPool.begin());
		EXPECT_EQ(1, memoryPool.size());
		EXPECT_EQ(1, *memoryPool.begin());

		EXPECT_NO_THROW(memoryPool.swap_with_back_and_pop_back(memoryPool.begin()));
		EXPECT_TRUE(memoryPool.empty());
	}

	TEST_F(Contiguous_memory_pool_test, swap)
	{
		Contiguous_memory_pool<int> memoryPool0(1);
		memoryPool0.emplace_back(1);

		Contiguous_memory_pool<int> memoryPool1(1);
		memoryPool1.emplace_back(2);

		ASSERT_NO_THROW(memoryPool0.swap(memoryPool1));

		EXPECT_EQ(*memoryPool0.begin(), 2);
		EXPECT_EQ(*memoryPool1.begin(), 1);
	}
}
