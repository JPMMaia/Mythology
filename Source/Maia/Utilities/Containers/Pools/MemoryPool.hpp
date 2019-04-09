#include <numeric>
#include <stdexcept>
#include <vector>

namespace Maia::Utilities
{
	template <class T>
	class Memory_pool;

	template <class T>
	class Memory_pool_const_iterator
	{
	public:

		// Public member types:
		using value_type = T;
		using reference = T&;
		using const_reference = T const&;
		using pointer = T*;

		// Constructors:
		Memory_pool_const_iterator(pointer element) :
			m_element(element)
		{
		}

		// Element access:
		const_reference operator*() const
		{
			return *m_element;
		}

	protected:

		// Friends:
		friend Memory_pool<value_type>;

		// Members:
		pointer m_element;

	};

	template <class T>
	class Memory_pool_iterator : public Memory_pool_const_iterator<T>
	{
	public:

		using pointer = typename Memory_pool_iterator<T>::pointer;
		using reference = typename Memory_pool_iterator<T>::reference;

		// Constructors:
		Memory_pool_iterator(pointer element) :
			Memory_pool_const_iterator<T>(element)
		{
		}

		// Element access:
		reference operator*() const
		{
			return *Memory_pool_const_iterator<T>::m_element;
		}

	};

	template <class T>
	class Memory_pool
	{
	public:

		// Public member types:
		using size_type = std::size_t;
		using value_type = T;
		using reference = T & ;
		using const_reference = const T&;
		using iterator = Memory_pool_iterator<T>;
		using const_iterator = Memory_pool_const_iterator<T>;

		// Constructors:
		Memory_pool() noexcept = default;
		Memory_pool(const Memory_pool& other) = delete;
		Memory_pool(Memory_pool&& other) noexcept :
			m_elements(std::move(other.m_elements)),
			m_inactive_elements(std::move(other.m_inactive_elements)),
			m_num_active_elements(other.m_num_active_elements)
		{
		}
		Memory_pool(size_type capacity) :
			m_elements(),
			m_inactive_elements(),
			m_num_active_elements(0)
		{
			reserve(capacity);
		}

		// Copy/move assignment:
		Memory_pool& operator=(const Memory_pool& other) = delete;
		Memory_pool& operator=(Memory_pool&& other) noexcept
		{
			m_elements = std::move(other.m_elements);
			m_inactive_elements = std::move(other.m_inactive_elements);
			m_num_active_elements = std::move(other.m_num_active_elements);
		}

		// Iterators:
		iterator begin()
		{
			return iterator(m_elements.data());
		}
		iterator end()
		{
			return iterator(m_elements.data() + size());
		}

		// capacity:
		bool empty() const noexcept
		{
			return size() == 0;
		}
		size_type size() const noexcept
		{
			return m_num_active_elements;
		}
		size_type max_size() const noexcept
		{
			return m_elements.max_size();
		}
		void reserve(size_type capacity)
		{
			m_elements.reserve(capacity);

			m_inactive_elements.resize(capacity);
			std::iota(m_inactive_elements.begin(), m_inactive_elements.end(), 0);
		}
		size_type capacity() const noexcept
		{
			return m_elements.capacity();
		}

		// Modifiers:
		void clear() noexcept
		{
			m_elements.clear();
			m_inactive_elements.clear();
			m_num_active_elements = 0;
		}
		template <class ...ArgumentsT>
		iterator emplace(ArgumentsT&&... arguments)
		{
			if (m_inactive_elements.empty())
			{
				throw std::out_of_range("Memory pool is full! Use the reserve method to reserve a block of memory!");
			}

			auto index = m_inactive_elements.back();
			m_inactive_elements.pop_back();
			++m_num_active_elements;

			m_elements[index] = value_type(std::forward<ArgumentsT>(arguments)...);

			return iterator(m_elements.data() + index);
		}
		void erase(const_iterator position)
		{
			--m_num_active_elements;
			m_inactive_elements.emplace_back(position.m_element - m_elements.data());

		}
		void swap(Memory_pool<value_type>& other) noexcept
		{
			std::swap(m_elements, other.m_elements);
			std::swap(m_inactive_elements, other.m_inactive_elements);
			std::swap(m_num_active_elements, other.m_num_active_elements);
		}

	private:

		// Friends:
		friend class const_iterator;
		friend class iterator;

		// Members:
		std::vector<value_type> m_elements;
		std::vector<size_type> m_inactive_elements;
		std::size_t m_num_active_elements = 0;

	};
}
