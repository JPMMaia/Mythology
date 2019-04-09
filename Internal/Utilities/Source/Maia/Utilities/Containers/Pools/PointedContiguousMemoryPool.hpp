#include <vector>

import Maia.Utilities.Memory.Contiguous_memory_pool;
import Maia.Utilities.Memory.Memory_pool;

export module Maia.Utilities.Memory.PointedContiguousMemoryPool;

namespace Maia
{
	namespace Utilities
	{
		template <class T>
		class PointedContiguousMemoryPool;

		export template <class T>
		class PointedContiguousMemoryPoolPointer
		{
		public:

			// Public member types:
			using value_type = T;
			using size_type = std::size_t;

			// Element Access:
			value_type& operator->() const
			{
				return *m_pointer;
			}
			value_type& operator*() const
			{
				return *m_pointer;
			}

		private:

			// Friends:
			friend PointedContiguousMemoryPool<T>;

			// Members:
			value_type* m_pointer;

		};

		export template <class T>
		class PointedContiguousMemoryPool
		{
		public:
			
			// Public member types:
			using size_type = std::size_t;
			using value_type = T;
			using reference = T&;
			using const_reference = const T&;
			using iterator = typename std::vector<T>::iterator;
			using const_iterator = typename std::vector<T>::const_iterator;
			using pointer = PointedContiguousMemoryPoolPointer<T>;

			// Constructors:
			PointedContiguousMemoryPool() noexcept = default;
			PointedContiguousMemoryPool(const PointedContiguousMemoryPool& other) = delete;
			PointedContiguousMemoryPool(PointedContiguousMemoryPool&& other) noexcept :
				m_elements(std::move(other.m_elements)),
				m_pointers(std::move(other.m_pointers))
			{
			}
			PointedContiguousMemoryPool(size_type capacity) :
				m_elements(),
				m_pointers()
			{
				reserve(capacity);
			}

			// Copy/move assignment:
			PointedContiguousMemoryPool& operator=(const PointedContiguousMemoryPool& other) = delete;
			PointedContiguousMemoryPool& operator=(PointedContiguousMemoryPool&& other) noexcept
			{
				m_elements = std::move(other.m_elements);
				m_pointers = std::move(other.m_pointers);
			}

			// Element Access:
			// TODO

			// Iterators:
			iterator begin()
			{
				return m_elements.begin();
			}
			iterator end()
			{
				return m_elements.end();
			}

			// capacity:
			bool empty() const noexcept
			{
				return m_elements.empty();
			}
			size_type size() const noexcept
			{
				return m_elements.size();
			}
			size_type max_size() const noexcept
			{
				return m_elements.max_size();
			}
			void reserve(size_type capacity)
			{
				m_elements.reserve(capacity);
				m_pointers.reserve(capacity);
			}
			size_type capacity() const noexcept
			{
				return m_elements.capacity();
			}

			// Modifiers:
			void clear() noexcept
			{
				m_elements.clear();
				m_pointers.clear();
			}
			template <class ...ArgumentsT>
			pointer emplace_back(ArgumentsT&&... arguments)
			{
				if (size() == capacity())
					throw std::out_of_range("Memory pool is full! Use the reserve method to reserve a block of memory!");

				return m_elements.emplace_back(std::forward<ArgumentsT>(arguments)...);
			}
			void swap_with_back_and_pop_back(iterator position)
			{
				// Overwrite element at given position by moving the element at the back:
				*position = std::move(m_elements[m_elements.size() - 1]);
				
				// Remove element at the back:
				m_elements.pop_back();
			}
			void swap(Contiguous_memory_pool& other) noexcept
			{
				std::swap(m_elements, other.m_elements);
			}

		private:

			// Members:
			Contiguous_memory_pool<value_type> m_elements;
			Memory_pool<pointer> m_pointers;

		};
	}
}
