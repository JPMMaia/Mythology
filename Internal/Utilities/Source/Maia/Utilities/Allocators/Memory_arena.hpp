#ifndef MAIA_UTILITIES_MEMORYARENA_H_INCLUDED
#define MAIA_UTILITIES_MEMORYARENA_H_INCLUDED

#include <cstddef>
#include <memory>
#include <stdexcept>

namespace Maia::Utilities
{
	class Memory_arena
	{
	public:

		explicit Memory_arena(std::size_t const size_in_bytes) :
			m_data{ std::make_unique<std::byte[]>(size_in_bytes) },
			m_size_in_bytes{ size_in_bytes },
			m_offset_in_bytes{ 0 }
		{
		}
		Memory_arena(Memory_arena const&) = delete;
		Memory_arena(Memory_arena&& other) = delete;

		Memory_arena& operator=(Memory_arena const&) = delete;
		Memory_arena& operator=(Memory_arena&&) = delete;

		void* allocate(std::size_t const size_in_bytes, std::size_t const alignment_in_bytes)
		{
			void* data = m_data.get() + m_offset_in_bytes;
			
			auto const original_space = m_size_in_bytes - m_offset_in_bytes;
			auto original_space_minus_alignment = original_space;

			if (!std::align(alignment_in_bytes, size_in_bytes, data, original_space_minus_alignment))
			{
				throw std::bad_alloc();
			}

			auto const bytes_used_for_alignment = original_space - original_space_minus_alignment;
			m_offset_in_bytes += size_in_bytes + bytes_used_for_alignment;

			return data;
		}
		void deallocate(void* const data, std::size_t const size_in_bytes) noexcept
		{
		}

		std::size_t capacity() const noexcept
		{
			return m_size_in_bytes;
		}
		std::size_t used_capacity() const noexcept
		{
			return m_offset_in_bytes;
		}

	private:

		std::unique_ptr<std::byte[]> const m_data;
		std::size_t const m_size_in_bytes;
		std::size_t m_offset_in_bytes;

	};
}

#endif
