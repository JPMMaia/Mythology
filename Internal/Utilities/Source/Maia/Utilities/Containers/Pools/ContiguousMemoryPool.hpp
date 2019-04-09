#include <cstddef>
#include <vector>

namespace Maia::Utilities
{
	template <class T>
	class Contiguous_memory_pool
	{
	public:

		// Public member types:
		using size_type = std::size_t;
		using value_type = T;
		using reference = T&;
		using const_reference = T const&;
		using iterator = typename std::vector<T>::iterator;
		using const_iterator = typename std::vector<T>::const_iterator;

		// Constructors:
		Contiguous_memory_pool() noexcept = default;
		Contiguous_memory_pool(const Contiguous_memory_pool& other) = delete;
		Contiguous_memory_pool(Contiguous_memory_pool&& other) noexcept :
			m_elements(std::move(other.m_elements))
		{
		}
		Contiguous_memory_pool(size_type capacity) :
			m_elements()
		{
			reserve(capacity);
		}

		// Copy/move assignment:
		Contiguous_memory_pool& operator=(Contiguous_memory_pool const& other) = delete;
		Contiguous_memory_pool& operator=(Contiguous_memory_pool&& other) noexcept
		{
			m_elements = std::move(other.m_elements);
		}

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
		}
		size_type capacity() const noexcept
		{
			return m_elements.capacity();
		}

		// Modifiers:
		void clear() noexcept
		{
			m_elements.clear();
		}
		template <class ...ArgumentsT>
		reference emplace_back(ArgumentsT&&... arguments)
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
		std::vector<value_type> m_elements;

	};
}
