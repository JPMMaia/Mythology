#include <cstddef>

#include <gtest/gtest.h>

#include <Maia/Utilities/Allocators/Forward_allocator.hpp>

namespace Maia::Utilities::Test
{
	class Forward_allocator_test : public ::testing::Test
	{
	};

	TEST_F(Forward_allocator_test, Should_allocate_using_memory_arena)
	{
		auto constexpr arena_capacity_in_bytes = 64;
		Memory_arena arena(arena_capacity_in_bytes);
		Forward_allocator<std::byte> allocator(arena);
		
		auto constexpr bytes_to_allocate = arena_capacity_in_bytes;
		auto* const data = allocator.allocate(arena_capacity_in_bytes);

		EXPECT_NE(data, nullptr);
		EXPECT_EQ(bytes_to_allocate, arena.used_capacity());

		allocator.deallocate(data, bytes_to_allocate);
	}

	TEST_F(Forward_allocator_test, Should_throw_bad_alloc_when_allocating_more_than_arena_capacity)
	{
		auto constexpr arena_capacity_in_bytes = 64;
		Memory_arena arena(arena_capacity_in_bytes);
		Forward_allocator<std::byte> allocator(arena);

		auto const allocate = [&]() 
		{ 
			auto const bytes_to_allocate = arena_capacity_in_bytes + 1;
			auto* const data = allocator.allocate(bytes_to_allocate); 
			allocator.deallocate(data, bytes_to_allocate);
		};
		ASSERT_THROW(allocate(), std::bad_alloc);
	}
}
