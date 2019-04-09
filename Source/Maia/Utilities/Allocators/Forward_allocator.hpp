#ifndef MAIA_UTILITIES_FORWARDALLOCATOR_H_INCLUDED
#define MAIA_UTILITIES_FORWARDALLOCATOR_H_INCLUDED

#include <cstddef>
#include <memory>
#include <type_traits>

#include <Maia/Utilities/Allocators/Memory_arena.hpp>

namespace Maia::Utilities
{
	template <class T>
	class Forward_allocator
	{
	public:

		template <class U> friend class Forward_allocator;

		using value_type = T;

		using propagate_on_container_copy_assignment = std::true_type;
		using propagate_on_container_move_assignment = std::true_type;
		using propagate_on_container_swap = std::true_type;

		explicit Forward_allocator(Memory_arena& memory_arena) noexcept
			: m_memory_arena(memory_arena)
		{
		}

		template <class U>
		constexpr Forward_allocator(Forward_allocator<U> const& other) noexcept
			: m_memory_arena(other.m_memory_arena)
		{
		}

		[[nodiscard]]
		T* allocate(std::size_t const num_elements)
		{
			auto const size_in_bytes = num_elements * sizeof(T);
			auto const alignment_in_bytes = alignof(T);

			auto* rawData = m_memory_arena.allocate(size_in_bytes, alignment_in_bytes);

			return static_cast<T*>(rawData);
		}

		void deallocate(T* const data, std::size_t const num_elements) noexcept
		{
			auto const size_in_bytes = num_elements * sizeof(T);

			m_memory_arena.deallocate(data, size_in_bytes);
		}

		template <class U>
		bool operator==(Forward_allocator<U> const& rhs) const noexcept
		{
			return &m_memory_arena == &rhs.m_memory_arena;
		}

	private:

		Memory_arena& m_memory_arena;

	};

	template <class T, class U>
	bool operator!=(Forward_allocator<T> const& lhs, Forward_allocator<U> const& rhs) noexcept
	{
		return !(lhs == rhs);
	}
}

#endif
