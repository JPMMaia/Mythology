export module maia.ecs.component_chunk_group;

import maia.ecs.component;
import maia.ecs.entity;
import maia.ecs.shared_component;

import <array>;
import <cassert>;
import <cstddef>;
import <iterator>;
import <memory_resource>;
import <optional>;
import <ostream>;
import <ranges>;
import <span>;
import <tuple>;
import <utility>;
import <vector>;
import <unordered_map>;

namespace Maia::ECS
{
    using Component_chunk = std::pmr::vector<std::byte>;


	export struct Component_group_entity_index
	{
		std::size_t value;
	};

	export struct Component_group_entity_moved
	{
		Entity entity;
	};

	export struct Component_type_info_and_offset
	{
		Component_type_ID id;
		std::size_t offset;
		Component_type_size size;
	};

	export using Chunk_group_hash = std::size_t;
	using Chunk = std::pmr::vector<std::byte>;

	struct Chunk_group
	{
		std::pmr::vector<Chunk> chunks;
		std::size_t number_of_elements;
	};

	std::size_t get_number_of_elements(
		std::size_t const number_of_elements_in_chunk_group,
		std::size_t const number_of_elements_per_chunk,
		std::size_t const chunk_index
	) noexcept
	{
		std::size_t const capacity_at_previous_chunk = chunk_index * number_of_elements_per_chunk;
		std::size_t const capacity_at_current_chunk = capacity_at_previous_chunk + number_of_elements_per_chunk;

		if (number_of_elements_in_chunk_group >= capacity_at_current_chunk)
		{
			return number_of_elements_per_chunk;
		}
		else if (number_of_elements_in_chunk_group > capacity_at_previous_chunk)
		{
			return number_of_elements_in_chunk_group - capacity_at_previous_chunk;
		}
		else
		{
			return 0;
		}
	}

	export template <typename Component_t>
	struct Component_view
	{
		using Pointer_type = std::conditional_t<std::is_const_v<Component_t>, std::byte const*, std::byte*>;

		Component_view(Pointer_type const pointer) noexcept :
			pointer{pointer}
		{
		}

		template <typename = typename std::enable_if<std::is_const_v<Component_t>>>
		Component_view(Component_view<std::remove_const_t<Component_t>> const& other) noexcept :
			pointer{other.pointer}
		{
		}

		operator Component_t() const noexcept
		{
			return read();
		}

		template <typename = typename std::enable_if<!std::is_const_v<Component_t>>>
		Component_view& operator=(Component_t const& component) const noexcept
		{
			write(component);
			
			return *this;
		}

		template <typename = typename std::enable_if<!std::is_const_v<Component_t>>>
		Component_view& operator=(Component_t&& component) const noexcept
		{
			write(component);

			return *this;
		}

		std::remove_const_t<Component_t> read() const noexcept
		{
			std::remove_const_t<Component_t> component;
			std::memcpy(&component, this->pointer, sizeof(Component_t));
			return component;
		}

		template <typename = typename std::enable_if<!std::is_const_v<Component_t>>>
		void write(std::remove_const_t<Component_t> const component) const noexcept
		{
			std::memcpy(this->pointer, &component, sizeof(Component_t));
		}

		Pointer_type pointer;
	};

	export template <typename T>
	std::ostream& operator<<(std::ostream& output_stream, Component_view<T> const view) noexcept
	{
		T const value = view;
		output_stream << value;
		
		return output_stream;
	}

	export template <typename Callable>
	auto call_with_tuple_arguments(Callable&& callable) noexcept
	{
		return [&callable] <typename Tuple> (Tuple&& tuple) noexcept
		{
			std::apply(std::forward<Callable>(callable), std::forward<Tuple>(tuple));
		};
	}

	template <typename Function, typename Tuple, typename Pointer_type, std::size_t... Indices>
	void apply(
		Function&& function,
		Tuple&& tuple,
		std::array<Pointer_type, sizeof...(Indices)> const& pointers,
		std::index_sequence<Indices...>
	) noexcept
	{
		(
			std::invoke(
				std::forward<Function>(function),
				std::get<Indices>(tuple),
				pointers[Indices]
			),
			...
		);
	};

	template <typename Function, typename Tuple, typename Pointer_type>
	void apply(
		Function&& function,
		Tuple&& tuple,
		std::array<Pointer_type, std::tuple_size_v<std::remove_reference_t<Tuple>>> const& pointers
	) noexcept
	{
		constexpr auto indices = std::make_index_sequence<std::tuple_size_v<std::remove_reference_t<Tuple>>>{};
		
		return apply(
			std::forward<Function>(function),
			std::forward<Tuple>(tuple),
			pointers,
			indices
		);
	}

	template <typename Function, std::size_t... Indices>
	void apply(
		Function&& function,
		std::index_sequence<Indices...>
	) noexcept
	{
		(
			std::invoke(
				std::forward<Function>(function),
				Indices
			),
			...
		);
	}

	template <typename Tuple, typename Function, typename... Ts>
	void apply(
		Function&& function,
		Ts&&... arguments
	) noexcept
	{
		constexpr auto indices = std::make_index_sequence<std::tuple_size_v<std::remove_reference_t<Tuple>>>{};

		auto const invoke = [&function, &arguments...] (std::size_t const index) -> void
		{
			std::invoke(
				std::forward<Function>(function),
				arguments[index]...
			);
		};

		apply(
			invoke,
			indices
		);
	}

	template <typename Type, typename Function, std::size_t... Indices> 
	std::array<Type, sizeof...(Indices)> create_array(Function&& generator, std::index_sequence<Indices...>) noexcept
	{
		return
		{
			generator(Indices)...
		};	
	};

	template <typename Type, std::size_t Size, typename Function> 
	std::array<Type, Size> create_array(Function&& generator) noexcept
	{
		constexpr auto indices = std::make_index_sequence<Size>{};

		return create_array<Type>(std::forward<Function>(generator), indices);
	};

	template <typename... Ts, typename Function, std::size_t... Indices> 
	std::tuple<Ts...> create_tuple(Function&& generator, std::index_sequence<Indices...>) noexcept
	{
		return std::make_tuple<Ts...>(
			generator(Indices)...
		);
	};

	template <typename... Ts, typename Function> 
	std::tuple<Ts...> create_tuple(Function&& generator) noexcept
	{
		constexpr auto indices = std::make_index_sequence<sizeof...(Ts)>{};

		return create_tuple<Ts...>(std::forward<Function>(generator), indices);
	};

	export template <typename... Ts>
	class Component_iterator
	{
	private:

		using First_type = std::tuple_element_t<0, std::tuple<Ts...>>;

	public:

		using difference_type = std::ptrdiff_t;
		using value_type = std::conditional_t<sizeof...(Ts) == 1, Component_view<First_type>, std::tuple<Component_view<Ts>...>>;
		using pointer = std::conditional_t<sizeof...(Ts) == 1, Component_view<First_type>*, std::tuple<Component_view<Ts>...>*>;
		using reference = std::conditional_t<sizeof...(Ts) == 1, Component_view<First_type>, std::tuple<Component_view<Ts>...>>;
		using iterator_category = std::random_access_iterator_tag;

		template<typename T>
		using Pointer_type = std::conditional_t<std::is_const_v<T>, std::byte const*, std::byte*>;

		using Pointers_type = std::conditional_t<
			sizeof...(Ts) == 1,
			Pointer_type<First_type>,
			std::tuple<Pointer_type<Ts>...>
		>;

		explicit Component_iterator(Pointers_type const data_pointers = {}) noexcept :
			m_views{data_pointers}
		{
		}

		template <typename = typename std::enable_if<std::conjunction_v<std::is_const<Ts>...>>>
		Component_iterator(Component_iterator<std::remove_const_t<Ts>...> const& other) noexcept :
			m_views{other.m_views}
		{
		}

		reference operator*() const noexcept
		{
			return m_views;
		}

		pointer operator->() const noexcept
		{
			return &m_views;
		}

		bool operator==(Component_iterator const rhs) const noexcept
		{
			if constexpr (sizeof...(Ts) == 1)
			{
				return m_views.pointer == rhs.m_views.pointer;
			}
			else
			{
				return std::get<0>(m_views).pointer == std::get<0>(rhs.m_views).pointer;
			}
		}

		Component_iterator& operator++() noexcept
		{
			if constexpr (sizeof...(Ts) == 1)
			{
				m_views.pointer += sizeof(First_type);
			}
			else
			{
				auto const increment_pointer = [] <typename T> (Component_view<T>& view, std::size_t const component_size)
				{
					view.pointer += component_size;
				};

				constexpr std::array<std::size_t, sizeof...(Ts)> component_sizes
				{
					sizeof(Ts)...
				};

				apply(increment_pointer, m_views, component_sizes);
			}

			return *this;
		}

		Component_iterator operator++(int) noexcept
		{
			Component_iterator const copy = *this;
			++(*this);
			return copy;
		}

		Component_iterator& operator--() noexcept
		{
			if constexpr (sizeof...(Ts) == 1)
			{
				m_views.pointer -= sizeof(First_type);
			}
			else
			{
				auto const decrement_pointer = [] <typename T> (Component_view<T>& view, std::size_t const component_size)
				{
					view.pointer -= component_size;
				};

				constexpr std::array<std::size_t, sizeof...(Ts)> component_sizes
				{
					sizeof(Ts)...
				};

				apply(decrement_pointer, m_views, component_sizes);
			}

			return *this;
		}

		Component_iterator operator--(int) noexcept
		{
			Component_iterator const copy = *this;
			--(*this);
			return copy;
		}

		bool operator<(Component_iterator const rhs) const noexcept
		{
			if constexpr (sizeof...(Ts) == 1)
			{
				return m_views.pointer < rhs.m_views.pointer;
			}
			else
			{
				return std::get<0>(m_views).pointer < std::get<0>(rhs.m_views).pointer;
			}
		}

		bool operator>(Component_iterator const rhs) const noexcept
		{
			if constexpr (sizeof...(Ts) == 1)
			{
				return m_views.pointer > rhs.m_views.pointer;
			}
			else
			{
				return std::get<0>(m_views).pointer > std::get<0>(rhs.m_views).pointer;
			}
		}

		bool operator<=(Component_iterator const rhs) const noexcept
		{
			if constexpr (sizeof...(Ts) == 1)
			{
				return m_views.pointer <= rhs.m_views.pointer;
			}
			else
			{
				return std::get<0>(m_views).pointer <= std::get<0>(rhs.m_views).pointer;
			}
		}

		bool operator>=(Component_iterator const rhs) const noexcept
		{
			if constexpr (sizeof...(Ts) == 1)
			{
				return m_views.pointer <= rhs.m_views.pointer;
			}
			else
			{
				return std::get<0>(m_views).pointer <= std::get<0>(rhs.m_views).pointer;
			}
		}

		difference_type operator-(Component_iterator const rhs) const noexcept
		{
			if constexpr (sizeof...(Ts) == 1)
			{
				assert((m_views.pointer - rhs.m_views.pointer) % sizeof(First_type) == 0);

				return (m_views.pointer - rhs.m_views.pointer) / sizeof(First_type);
			}
			else
			{
				assert((std::get<0>(m_views).pointer - std::get<0>(rhs.m_views).pointer) % sizeof(First_type) == 0);

				return (std::get<0>(m_views).pointer - std::get<0>(rhs.m_views).pointer) / sizeof(First_type);
			}
		}

		Component_iterator& operator+=(difference_type const n) noexcept
		{
			if constexpr (sizeof...(Ts) == 1)
			{
				m_views.pointer += n * sizeof(First_type);
			}
			else
			{
				auto const increment_pointer = [n] <typename T> (Component_view<T>& view, std::size_t const component_size)
				{
					view.pointer += n * component_size;
				};

				constexpr std::array<std::size_t, sizeof...(Ts)> component_sizes
				{
					sizeof(Ts)...
				};

				apply(increment_pointer, m_views, component_sizes);
			}
			
			return *this;
		}

		Component_iterator operator+(difference_type const n) const noexcept
		{
			Component_iterator copy = *this;
			copy += n;
			return copy;
		}

		Component_iterator& operator-=(difference_type const n) noexcept
		{
			if constexpr (sizeof...(Ts) == 1)
			{
				m_views.pointer -= n * sizeof(First_type);
			}
			else
			{
				auto const decrement_pointer = [n] <typename T> (Component_view<T>& view, std::size_t const component_size)
				{
					view.pointer -= n * component_size;
				};

				constexpr std::array<std::size_t, sizeof...(Ts)> component_sizes
				{
					sizeof(Ts)...
				};

				apply(decrement_pointer, m_views, component_sizes);
			}
		}

		Component_iterator operator-(difference_type const n) const noexcept
		{
			Component_iterator copy = *this;
			copy -= n;
			return copy;
		}

		reference operator[](difference_type const n) const noexcept
		{
			if constexpr (sizeof...(Ts) == 1)
			{
				Component_view<First_type> view = m_views;
				view.pointer += n * sizeof(First_type);
				return view;
			}
			else
			{
				std::tuple<Component_view<Ts>...> views = m_views;

				auto const increment_pointer = [n] <typename T> (Component_view<T>& view, std::size_t const component_size)
				{
					view.pointer += n * component_size;
				};

				constexpr std::array<std::size_t, sizeof...(Ts)> component_sizes
				{
					sizeof(Ts)...
				};

				apply(increment_pointer, views, component_sizes);

				return views;
			}
		}

	private:

		friend Component_iterator<Ts const...>;

		value_type m_views;

	};

	export template <typename... Ts>
	Component_iterator<Ts...> operator+(typename Component_iterator<Ts...>::difference_type const lhs, Component_iterator<Ts...> const rhs) noexcept
	{
		return rhs + lhs;
	}

	export template <typename... Ts>
	class Component_chunk_view : public std::ranges::view_base
	{
	public:

		using Iterator = Component_iterator<Ts...>;

		Component_chunk_view() noexcept = default;

		template <typename = typename std::enable_if<std::conjunction_v<std::is_const<Ts>...>>>
		Component_chunk_view(Component_chunk_view<std::remove_const_t<Ts>...> const& other) noexcept :
			m_begin{other.begin()},
			m_end{other.end()}
		{
		}

		Component_chunk_view(Iterator const begin, Iterator const end) noexcept :
			m_begin{begin},
			m_end{end}
		{
		}

		Iterator begin() const noexcept
		{
			return m_begin;
		}

		Iterator end() const noexcept
		{
			return m_end;
		}

	private:

		Iterator m_begin{};
		Iterator m_end{};
	};

	export template <typename... Ts>
	class Component_chunk_iterator
	{
	public:

		using difference_type = std::ptrdiff_t;
		using value_type = Component_chunk_view<Ts...>;
		using pointer = Component_chunk_view<Ts...> const*;
		using reference = Component_chunk_view<Ts...> const&;
		using iterator_category = std::bidirectional_iterator_tag;

		using Chunk_group_type =
			std::conditional_t<
				std::conjunction_v<std::is_const<Ts>...>,
				Chunk_group const,
				Chunk_group
			>;

		Component_chunk_iterator() noexcept = default;

		template <typename = typename std::enable_if<std::conjunction_v<std::is_const<Ts>...>>>
		Component_chunk_iterator(Component_chunk_iterator<std::remove_const_t<Ts>...> const& other) noexcept :
			m_chunk_group{other.m_chunk_group},
			m_current_chunk_index{other.m_current_chunk_index},
			m_number_of_elements_per_chunk{other.m_number_of_elements_per_chunk},
			m_component_offsets{other.m_component_offsets},
			m_component_chunk_view{other.m_component_chunk_view}
		{
		}

		Component_chunk_iterator(
			Chunk_group_type& chunk_group,
			std::size_t const chunk_index,
			std::size_t const number_of_elements_per_chunk,
			std::array<std::size_t, sizeof...(Ts)> const component_offsets
		) noexcept :
			m_chunk_group{&chunk_group},
			m_current_chunk_index{chunk_index},
			m_number_of_elements_per_chunk{number_of_elements_per_chunk},
			m_component_offsets{component_offsets},
			m_component_chunk_view{create_component_view()}
		{
		}

		reference operator*() const noexcept
		{
			return m_component_chunk_view;
		}

		pointer operator->() const noexcept
		{
			return &m_component_chunk_view;
		}

		bool operator==(Component_chunk_iterator const rhs) const noexcept
		{
			return m_chunk_group == rhs.m_chunk_group
				&& m_current_chunk_index == rhs.m_current_chunk_index;
		}

		Component_chunk_iterator& operator++() noexcept
		{
			++m_current_chunk_index;
			m_component_chunk_view = create_component_view();

			return *this;
		}

		Component_chunk_iterator operator++(int) noexcept
		{
			Component_chunk_iterator const copy = *this;
			++(*this);
			return copy;
		}

		Component_chunk_iterator& operator--() noexcept
		{
			--m_current_chunk_index;
			m_component_chunk_view = create_component_view();
			return *this;
		}

		Component_chunk_iterator operator--(int) noexcept
		{
			Component_chunk_iterator const copy = *this;
			--(*this);
			return copy;
		}

		difference_type operator-(Component_chunk_iterator const rhs) const noexcept
		{
			assert(m_chunk_group == rhs.m_chunk_group);

			if (m_current_chunk_index >= rhs.m_current_chunk_index) [[likely]]
			{
				return m_current_chunk_index - rhs.m_current_chunk_index;
			}
			else
			{
				return -static_cast<difference_type>(rhs.m_current_chunk_index - m_current_chunk_index);
			}
		}

		std::size_t get_number_of_elements_per_chunk() const noexcept
		{
			return m_number_of_elements_per_chunk;
		}

		std::array<std::size_t, sizeof...(Ts)> get_component_offsets() const noexcept
		{
			return m_component_offsets;
		}

	private:

		Component_chunk_view<Ts...> create_component_view() const noexcept
		{
			std::size_t const number_of_elements_in_chunk = get_number_of_elements(
				m_chunk_group->number_of_elements,
				m_number_of_elements_per_chunk,
				m_current_chunk_index
			);

			if (number_of_elements_in_chunk != 0) [[likely]]
			{
				using Pointer_type = 
					std::conditional_t<
						std::conjunction_v<std::is_const<Ts>...>,
						std::byte const*,
						std::byte*
					>;

				using Chunk_type =
					std::conditional_t<
						std::conjunction_v<std::is_const<Ts>...>,
						Chunk const,
						Chunk
					>;

				Chunk_type& chunk = m_chunk_group->chunks[m_current_chunk_index];

				if constexpr (sizeof...(Ts) == 1)
				{
					Component_iterator<Ts...> const chunk_begin
					{
						chunk.data() + m_component_offsets[0]
					};

					using T = std::tuple_element_t<0, std::tuple<Ts...>>;

					Component_iterator<Ts...> const chunk_end
					{
						chunk.data() + m_component_offsets[0] + number_of_elements_in_chunk * sizeof(T)
					};

					return {chunk_begin, chunk_end};
				}
				else
				{
					auto const create_begin_pointer = [this, &chunk] (std::size_t const index) -> Pointer_type
					{
						return chunk.data() + m_component_offsets[index];
					};

					Component_iterator<Ts...> const chunk_begin
					{
						create_tuple<std::conditional_t<std::is_const_v<Ts>, std::byte const*, std::byte*>...>(
							create_begin_pointer
						)
					};

					constexpr std::array<std::size_t, sizeof...(Ts)> component_sizes
					{
						sizeof(Ts)...
					};

					auto const create_end_pointer = [this, &chunk, number_of_elements_in_chunk, &component_sizes] (std::size_t const index) -> Pointer_type
					{
						return chunk.data() + m_component_offsets[index] + number_of_elements_in_chunk * component_sizes[index];
					};

					Component_iterator<Ts...> const chunk_end
					{
						create_tuple<std::conditional_t<std::is_const_v<Ts>, std::byte const*, std::byte*>...>(
							create_end_pointer
						)
					};

					return {chunk_begin, chunk_end};
				}
			}
			else
			{
				return {};
			}
		}
		
		friend Component_chunk_iterator<Ts const...>;

		Chunk_group_type* m_chunk_group;
		std::size_t m_current_chunk_index;
		std::size_t m_number_of_elements_per_chunk;
		std::array<std::size_t, sizeof...(Ts)> m_component_offsets;
		Component_chunk_view<Ts...> m_component_chunk_view;

	};

	export template <typename... Ts>
	class Component_chunk_group_view : public std::ranges::view_base
	{
	public:

		using Iterator = Component_chunk_iterator<Ts...>;

		Component_chunk_group_view() noexcept = default;

		template <typename = typename std::enable_if<std::conjunction_v<std::is_const<Ts>...>>>
		Component_chunk_group_view(Component_chunk_group_view<std::remove_const_t<Ts>...> const& other) noexcept :
			m_begin{other.begin()},
			m_end{other.end()}
		{
		}

		Component_chunk_group_view(Iterator const begin, Iterator const end) noexcept :
			m_begin{begin},
			m_end{end}
		{
		}

		Iterator begin() const noexcept
		{
			return m_begin;
		}

		Iterator end() const noexcept
		{
			return m_end;
		}

	private:

		Iterator m_begin;
		Iterator m_end;

	};

	export template <typename... Ts>
	class Component_chunk_group_iterator
	{
	public:

		using difference_type = std::ptrdiff_t;
		using value_type = Component_chunk_group_view<Ts...>;
		using pointer = Component_chunk_group_view<Ts...> const*;
		using reference = Component_chunk_group_view<Ts...> const&;
		using iterator_category = std::bidirectional_iterator_tag;

		using Chunk_group_iterator = 
			std::conditional_t<
				std::conjunction_v<std::is_const<Ts>...>,
				std::pmr::unordered_map<Chunk_group_hash, Chunk_group>::const_iterator,
				std::pmr::unordered_map<Chunk_group_hash, Chunk_group>::iterator
			>;

		Component_chunk_group_iterator() noexcept = default;

		template <typename = typename std::enable_if<std::conjunction_v<std::is_const<Ts>...>>>
		Component_chunk_group_iterator(Component_chunk_group_iterator<std::remove_const_t<Ts>...> const& other) noexcept :
			m_current{other.m_current},
			m_end{other.m_end},
			m_component_chunk_group_view{other.m_component_chunk_group_view}
		{
		}

		Component_chunk_group_iterator(
			Chunk_group_iterator const begin,
			Chunk_group_iterator const end,
			std::size_t const number_of_elements_per_chunk,
			std::array<std::size_t, sizeof...(Ts)> const component_offsets
		) noexcept :
			m_current{begin},
			m_end{end},
			m_component_chunk_group_view{create_component_group_view(number_of_elements_per_chunk, component_offsets)}
		{
		}

		reference operator*() const noexcept
		{
			return m_component_chunk_group_view;
		}

		pointer operator->() const noexcept
		{
			return &m_component_chunk_group_view;
		}

		bool operator==(Component_chunk_group_iterator const rhs) const noexcept
		{
			return m_current == rhs.m_current;
		}

		Component_chunk_group_iterator& operator++() noexcept
		{
			++m_current;
			m_component_chunk_group_view = create_component_group_view();

			return *this;
		}

		Component_chunk_group_iterator operator++(int) noexcept
		{
			Component_chunk_group_iterator const copy = *this;
			++(*this);
			return copy;
		}

		Component_chunk_group_iterator& operator--() noexcept
		{
			--m_current;
			m_component_chunk_group_view = create_component_group_view();
			return *this;
		}

		Component_chunk_group_iterator operator--(int) noexcept
		{
			Component_chunk_group_iterator const copy = *this;
			--(*this);
			return copy;
		}

		difference_type operator-(Component_chunk_group_iterator const rhs) const noexcept
		{
			return m_current - rhs.m_current;
		}

	private:

		Component_chunk_group_view<Ts...> create_component_group_view(
			std::size_t const number_of_elements_per_chunk,
			std::array<std::size_t, sizeof...(Ts)> const& component_offsets
		) const noexcept
		{
			if (m_current != m_end) [[likely]]
			{
				using Chunk_group_type = typename Component_chunk_iterator<Ts...>::Chunk_group_type;

				Chunk_group_type& chunk_group = m_current->second;

				Component_chunk_iterator<Ts...> const begin
				{
					chunk_group,
					0,
					number_of_elements_per_chunk,
					component_offsets
				};

				Component_chunk_iterator<Ts...> const end
				{
					chunk_group,
					chunk_group.number_of_elements,
					number_of_elements_per_chunk,
					component_offsets
				};

				return {begin, end};
			}
			else
			{
				return {};
			}			
		}

		Component_chunk_group_view<Ts...> create_component_group_view() const noexcept
		{
			assert(m_component_chunk_group_view.begin() != m_component_chunk_group_view.end());

			auto const iterator = m_component_chunk_group_view.begin();

			std::size_t const number_of_elements_per_chunk = 
				iterator.get_number_of_elements_per_chunk();

			std::array<std::size_t, sizeof...(Ts)> const component_offsets =
				iterator.get_component_offsets();

			return create_component_group_view(number_of_elements_per_chunk, component_offsets);
		}

		friend Component_chunk_group_iterator<Ts const...>;
		
		Chunk_group_iterator m_current;
		Chunk_group_iterator m_end;
		Component_chunk_group_view<Ts...> m_component_chunk_group_view;

	};

	export struct Entity_move_result
	{
		std::size_t new_index;
		std::optional<Component_group_entity_moved> entity_moved_by_remove;
	};

	export template <typename... Ts>
	class Component_chunk_group_all_view : public std::ranges::view_base
	{
	public:

		using Iterator = Component_chunk_group_iterator<Ts...>;

		Component_chunk_group_all_view() noexcept = default;

		template <typename = typename std::enable_if<std::conjunction_v<std::is_const<Ts>...>>>
		Component_chunk_group_all_view(Component_chunk_group_all_view<std::remove_const_t<Ts>...> const& other) noexcept :
			m_begin{other.begin()},
			m_end{other.end()}
		{
		}

		Component_chunk_group_all_view(Iterator const begin, Iterator const end) noexcept :
			m_begin{begin},
			m_end{end}
		{
		}

		Iterator begin() const noexcept
		{
			return m_begin;
		}

		Iterator end() const noexcept
		{
			return m_end;
		}

	private:

		Iterator m_begin;
		Iterator m_end;

	};

	export class Component_chunk_group
	{
	public:

		using Index = std::size_t;

		Component_chunk_group(
			std::span<Component_type_info const> const component_type_infos,
			std::size_t const number_of_entities_per_chunk,
			std::pmr::polymorphic_allocator<std::byte> const& chunk_allocator,
			std::pmr::polymorphic_allocator<std::byte> const& allocator
		) noexcept :
			m_chunk_groups{allocator},
			m_component_type_infos{allocator},
			m_number_of_entities_per_chunk{number_of_entities_per_chunk},
			m_chunk_allocator{chunk_allocator}
		{
			m_component_type_infos.reserve(component_type_infos.size() + 1);
			m_component_type_infos.assign(component_type_infos.begin(), component_type_infos.end());
			m_component_type_infos.push_back({get_component_type_id<Entity>(), sizeof(Entity)});
		}

		Component_chunk_group(
			std::span<Component_type_ID const> const component_type_ids,
			std::span<Component_type_size const> const component_type_sizes,
			std::size_t const number_of_entities_per_chunk,
			std::pmr::polymorphic_allocator<std::byte> const& chunk_allocator,
			std::pmr::polymorphic_allocator<std::byte> const& allocator
		) noexcept :
			m_chunk_groups{allocator},
			m_component_type_infos{allocator},
			m_number_of_entities_per_chunk{number_of_entities_per_chunk},
			m_chunk_allocator{chunk_allocator}
		{
			assert(component_type_ids.size() == component_type_sizes.size());

			auto const to_component_type_info = [] (Component_type_ID const id, Component_type_size const size) -> Component_type_info
			{
				return {id, size};
			};

			m_component_type_infos.resize(component_type_ids.size() + 1);

			for (std::size_t component_type_index = 0; component_type_index < component_type_ids.size(); ++component_type_index)
			{
				m_component_type_infos[component_type_index] = to_component_type_info(
					component_type_ids[component_type_index],
					component_type_sizes[component_type_index]
				);
			}
			
			m_component_type_infos.back() = {get_component_type_id<Entity>(), sizeof(Entity)};
		}

		Component_chunk_group(
			std::span<Component_type_info const> const component_type_infos,
			std::size_t const number_of_entities_per_chunk,
			std::span<Chunk_group_hash const> const group_hashes,
			std::span<std::size_t const> const number_of_entities_per_group,
			std::pmr::polymorphic_allocator<std::byte> const& chunk_allocator,
			std::pmr::polymorphic_allocator<std::byte> const& allocator
		) noexcept :
			Component_chunk_group(component_type_infos, number_of_entities_per_chunk, chunk_allocator, allocator)
		{
			allocate_generic_memory(
				group_hashes,
				number_of_entities_per_group,
				number_of_entities_per_chunk
			);
		}

		Index add_entity(Entity const entity, Chunk_group_hash const chunk_group_hash)
		{
			auto const location = m_chunk_groups.find(chunk_group_hash);

			if (location != m_chunk_groups.end())
			{
				Chunk_group& chunk_group = location->second;

				std::size_t const chunk_group_capacity = m_number_of_entities_per_chunk * chunk_group.chunks.size();

				if (chunk_group.number_of_elements < chunk_group_capacity)
				{
					std::size_t const chunk_index = chunk_group.number_of_elements / m_number_of_entities_per_chunk;
					Chunk& chunk = chunk_group.chunks.at(chunk_index);

					std::size_t const new_entity_index = chunk_group.number_of_elements;

					{
						std::size_t entity_value_offset = get_component_element_offset(get_entity_component_type_info(), new_entity_index);

						assert((entity_value_offset + sizeof(Entity)) <= chunk.size());
						std::memcpy(chunk.data() + entity_value_offset, &entity, sizeof(Entity));
					}

					for (std::size_t component_type_index = 0; component_type_index + 1 < m_component_type_infos.size(); ++component_type_index)
					{
						Component_type_info const type_info = m_component_type_infos[component_type_index];

						std::size_t component_value_offset = get_component_element_offset(type_info, new_entity_index);

						assert((component_value_offset + type_info.size) <= chunk.size());
						std::memset(chunk.data() + component_value_offset, 0, type_info.size);
					}
					
					++chunk_group.number_of_elements;

					return chunk_group.number_of_elements - 1;
				}
				else
				{
					Chunk new_chunk{m_chunk_allocator};
					new_chunk.resize(get_chunk_size());
					
					std::size_t const new_entity_index = chunk_group.number_of_elements;

					{
						std::size_t entity_value_offset = get_component_element_offset(get_entity_component_type_info(), new_entity_index);

						assert((entity_value_offset + sizeof(Entity)) <= new_chunk.size());
						std::memcpy(new_chunk.data() + entity_value_offset, &entity, sizeof(Entity));
					}

					for (std::size_t component_type_index = 0; component_type_index + 1 < m_component_type_infos.size(); ++component_type_index)
					{
						Component_type_info const type_info = m_component_type_infos[component_type_index];

						std::size_t component_value_offset = get_component_element_offset(type_info, new_entity_index);

						assert((component_value_offset + type_info.size) <= new_chunk.size());
						std::memset(new_chunk.data() + component_value_offset, 0, type_info.size);
					}

					++chunk_group.number_of_elements;

					chunk_group.chunks.push_back(std::move(new_chunk));

					return chunk_group.number_of_elements - 1;
				}
			}
			else
			{
				Chunk new_chunk{m_chunk_allocator};
				new_chunk.resize(get_chunk_size());
								
				std::size_t const new_entity_index = 0;

				{
					std::size_t entity_value_offset = get_component_element_offset(get_entity_component_type_info(), new_entity_index);

					assert((entity_value_offset + sizeof(Entity)) <= new_chunk.size());
					std::memcpy(new_chunk.data() + entity_value_offset, &entity, sizeof(Entity));
				}

				for (std::size_t component_type_index = 0; component_type_index + 1 < m_component_type_infos.size(); ++component_type_index)
				{
					Component_type_info const type_info = m_component_type_infos[component_type_index];

					std::size_t component_value_offset = get_component_element_offset(type_info, new_entity_index);

					assert((component_value_offset + type_info.size) <= new_chunk.size());
					std::memset(new_chunk.data() + component_value_offset, 0, type_info.size);
				}

				std::pmr::vector<Chunk> chunks{m_chunk_groups.get_allocator()};
				chunks.push_back(std::move(new_chunk));

				Chunk_group chunk_group{std::move(chunks), 1};

				m_chunk_groups.emplace(chunk_group_hash, std::move(chunk_group));

				return 0;
			}
		}

		std::optional<Component_group_entity_moved> remove_entity(Chunk_group_hash const chunk_group_hash, Index const index) noexcept
		{
			Chunk_group& chunk_group = m_chunk_groups.at(chunk_group_hash);
			return remove_entity(chunk_group, index);
		}

		Entity_move_result move_entity(
			Chunk_group_hash const from_chunk_group_hash,
			Index const from_index,
			Chunk_group_hash const to_chunk_group_hash
		) noexcept
		{
			Chunk_group& from_group = m_chunk_groups.at(from_chunk_group_hash);

			Entity const entity = get_component_value<Entity>(from_group, from_index);

			Index const to_index = add_entity(entity, to_chunk_group_hash);

			Chunk_group& to_group = m_chunk_groups.at(to_chunk_group_hash);

			copy_components(from_group, from_index, to_group, to_index);

			std::optional<Component_group_entity_moved> const entity_moved_from_remove = 
				remove_entity(from_group, from_index);

			return {to_index, entity_moved_from_remove};
		}

		Entity get_entity(Chunk_group_hash const chunk_group_hash, Index const index) const noexcept
		{
			return get_component_value<Entity>(chunk_group_hash, index);
		}

		template <Concept::Component Component_t>
		Component_t get_component_value(Chunk_group_hash const chunk_group_hash, Index const index) const noexcept
		{
			Chunk_group const& chunk_group = m_chunk_groups.at(chunk_group_hash);

			return get_component_value<Component_t>(chunk_group, index);
		}

		template <Concept::Component Component_t>
		void set_component_value(Chunk_group_hash const chunk_group_hash, Index const index, Component_t const& value) noexcept
		{
			Chunk_group& chunk_group = m_chunk_groups.at(chunk_group_hash);
			assert(index < chunk_group.number_of_elements);

			std::size_t const chunk_index = index / m_number_of_entities_per_chunk;
			Chunk& chunk = chunk_group.chunks[chunk_index];

			Component_type_info const component_type_info{get_component_type_id<Component_t>(), sizeof(Component_t)};
			std::size_t const offset = get_component_element_offset(component_type_info, index);

			assert((offset + component_type_info.size) <= chunk.size());
			std::memcpy(chunk.data() + offset, &value, component_type_info.size);
		}

		void shrink_to_fit(Chunk_group_hash const chunk_group_hash) noexcept
		{
			Chunk_group& chunk_group = m_chunk_groups.at(chunk_group_hash);

			std::size_t const minimal_number_of_chunks = 
				chunk_group.number_of_elements / m_number_of_entities_per_chunk +
				(chunk_group.number_of_elements % m_number_of_entities_per_chunk != 0 ? 1 : 0);

			chunk_group.chunks.resize(minimal_number_of_chunks);
		}

		bool has_component_type(Component_type_ID const id) const noexcept
		{
			auto const is_component_type = [&id](Component_type_info const info) -> bool
			{
				return info.id == id;
			};

			auto const location = std::find_if(
				m_component_type_infos.begin(),
				m_component_type_infos.end(),
				is_component_type
			);

			return location != m_component_type_infos.end();
		}

		std::size_t number_of_entities() const noexcept
		{
			std::size_t count = 0;

			for (std::pair<Chunk_group_hash, Chunk_group> const& chunk_group : m_chunk_groups)
			{
				count += chunk_group.second.number_of_elements;
			}

			return count;
		}

		std::size_t number_of_entities(Chunk_group_hash const chunk_group_hash) const noexcept
		{
			auto const chunk_iterator = m_chunk_groups.find(chunk_group_hash);

			return chunk_iterator != m_chunk_groups.end() ?
					chunk_iterator->second.number_of_elements :
					0;			
		}

		std::size_t number_of_chunks() const noexcept
		{
			std::size_t count = 0;

			for (std::pair<Chunk_group_hash, Chunk_group> const& chunk_group : m_chunk_groups)
			{
				count += chunk_group.second.chunks.size();
			}

			return count;
		}

		std::size_t number_of_chunks(Chunk_group_hash const chunk_group_hash) const noexcept
		{
			auto const location = m_chunk_groups.find(chunk_group_hash);

			if (location != m_chunk_groups.end())
			{				Chunk_group const& chunk_group = m_chunk_groups.at(chunk_group_hash);

				return chunk_group.chunks.size();
			}
			else
			{
				return 0;
			}
		}

		template <Concept::Component... Component_ts>
		Component_chunk_view<Component_ts const...> get_view(Chunk_group_hash const chunk_group_hash, std::size_t const chunk_index) const noexcept
		{
			using Self = std::remove_reference_t<decltype(*this)>;

			return get_view<Self, Component_ts...>(this, chunk_group_hash, chunk_index);
		}

		template <Concept::Component... Component_ts>
		Component_chunk_view<Component_ts...> get_view(Chunk_group_hash const chunk_group_hash, std::size_t const chunk_index) noexcept
		{
			using Self = std::remove_reference_t<decltype(*this)>;

			return get_view<Self, Component_ts...>(this, chunk_group_hash, chunk_index);
		}

		template <Concept::Component... Component_ts>
		Component_chunk_group_view<Component_ts const...> get_view(Chunk_group_hash const chunk_group_hash) const noexcept
		{
			using Self = std::remove_reference_t<decltype(*this)>;

			return get_view<Self, Component_ts...>(this, chunk_group_hash);
		}

		template <Concept::Component... Component_ts>
		Component_chunk_group_view<Component_ts...> get_view(Chunk_group_hash const chunk_group_hash) noexcept
		{
			using Self = std::remove_reference_t<decltype(*this)>;

			return get_view<Self, Component_ts...>(this, chunk_group_hash);
		}

		template <Concept::Component... Component_ts>
		Component_chunk_group_all_view<Component_ts const...> get_view() const noexcept
		{
			using Self = std::remove_reference_t<decltype(*this)>;

			return get_view<Self, Component_ts...>(this);
		}

		template <Concept::Component... Component_ts>
		Component_chunk_group_all_view<Component_ts...> get_view() noexcept
		{
			using Self = std::remove_reference_t<decltype(*this)>;

			return get_view<Self, Component_ts...>(this);
		}

		static constexpr std::size_t get_instance_required_memory_size(
			std::size_t const number_of_component_types
		) noexcept
		{
			return 512;
		}

		static constexpr std::size_t get_new_chunk_group_required_memory_size() noexcept
		{
			return sizeof(decltype(m_chunk_groups)::value_type);
		}

		static constexpr std::size_t get_new_chunk_required_memory_size() noexcept
		{
			return sizeof(Chunk); 
		}

		static constexpr std::size_t get_components_required_memory_size(
			std::span<Component_type_size const> const component_type_sizes,
			std::size_t const number_of_entities_per_chunk
		) noexcept
		{
			std::size_t total_component_size = 
				std::accumulate(
					component_type_sizes.begin(),
					component_type_sizes.end(),
					sizeof(Entity)
				);

			std::size_t const total_size = total_component_size * number_of_entities_per_chunk;

			return total_size;
		}

		static constexpr std::size_t get_required_generic_memory_size(
			std::span<std::size_t const> const number_of_entities_per_group,
			std::size_t const number_of_entities_per_chunk,
			std::span<Component_type_size const> const component_type_sizes
		) noexcept
		{
			std::size_t const instance_required_size = get_instance_required_memory_size(component_type_sizes.size());

			std::size_t const groups_required_size = get_new_chunk_group_required_memory_size() * number_of_entities_per_group.size();

			std::size_t chunks_size = 0;

			for (std::size_t const number_of_entities_in_group : number_of_entities_per_group)
			{
				std::size_t const number_of_chunks_in_group =
					number_of_entities_in_group / number_of_entities_per_chunk +
					(number_of_entities_in_group % number_of_entities_per_chunk == 0 ? 0 : 1);

				chunks_size += number_of_chunks_in_group * get_new_chunk_required_memory_size();
			}

			std::size_t const total_size = instance_required_size + groups_required_size + chunks_size;

			return 3000;
		}

	private:

		void allocate_generic_memory(
			std::span<Chunk_group_hash const> const group_hashes,
			std::span<std::size_t const> const number_of_entities_per_group,
			std::size_t const number_of_entities_per_chunk		)
		{
			assert(group_hashes.size() == number_of_entities_per_group.size());

			m_chunk_groups.reserve(group_hashes.size());

			for (std::size_t group_index = 0; group_index < group_hashes.size(); ++group_index)
			{
				Chunk_group_hash const group_hash = group_hashes[group_index];
				assert(!m_chunk_groups.contains(group_hash));

				std::size_t const number_of_entities_in_group = number_of_entities_per_group[group_index];

				std::size_t const number_of_chunks_in_group =
					number_of_entities_in_group / number_of_entities_per_chunk +
					(number_of_entities_in_group % number_of_entities_per_chunk == 0 ? 0 : 1);

				std::pmr::vector<Chunk> chunks{m_chunk_groups.get_allocator()};
				Chunk_group group = {std::move(chunks)};
				group.chunks.reserve(number_of_chunks_in_group);
				m_chunk_groups.emplace(group_hash, std::move(group));
			}
		}

		std::size_t number_of_elements(std::size_t const number_of_elements_in_chunk_group, std::size_t const chunk_index) const noexcept
		{
			std::size_t const capacity_at_previous_chunk = chunk_index * m_number_of_entities_per_chunk;
			std::size_t const capacity_at_current_chunk = capacity_at_previous_chunk + m_number_of_entities_per_chunk;

			if (number_of_elements_in_chunk_group >= capacity_at_current_chunk)
			{
				return m_number_of_entities_per_chunk;
			}
			else if (number_of_elements_in_chunk_group > capacity_at_previous_chunk)
			{
				return number_of_elements_in_chunk_group - capacity_at_previous_chunk;
			}
			else
			{
				return 0;
			}
		}

		std::size_t get_chunk_size() const noexcept
		{
			std::size_t const total_component_size = [this]
			{
				std::size_t total_component_size = 0;

				for (Component_type_info const& type_info : m_component_type_infos)
				{
					total_component_size += type_info.size;
				}

				return total_component_size;
			}();

			return m_number_of_entities_per_chunk * total_component_size;
		}

		std::size_t get_component_offset(Component_type_ID const component_type_id) const noexcept
		{
			auto const is_type_info = [&component_type_id](Component_type_info const type_info) -> bool
			{
				return type_info.id == component_type_id;
			};

			auto const location = std::find_if(
				m_component_type_infos.begin(),
				m_component_type_infos.end(),
				is_type_info
			);

			std::ptrdiff_t const target_type_info_index = std::distance(m_component_type_infos.begin(), location);

			std::size_t offset = 0;

			for (std::size_t type_info_index = 0; type_info_index < target_type_info_index; ++type_info_index)
			{
				Component_type_info const& type_info = m_component_type_infos[type_info_index];
				offset += type_info.size;
			}

			return offset;
		}

		std::size_t get_entity_component_chunk_offset() const noexcept
		{
			return get_component_offset(get_component_type_id<Entity>());
		}

		std::size_t get_component_element_offset(
			Component_type_info const component_type_info,
			std::size_t entity_index
		) const noexcept
		{
			Component_type_ID const component_type_id = component_type_info.id;
			std::size_t const component_type_size = component_type_info.size;

			std::size_t const offset = 
				m_number_of_entities_per_chunk * get_component_offset(component_type_id) + 
				(entity_index % m_number_of_entities_per_chunk) * component_type_size;

			return offset;
		}

		Component_type_info get_entity_component_type_info() const noexcept
		{
			return m_component_type_infos.back();
		}

		template <typename Self, Concept::Component... Component_ts>
		static auto get_view(
			Self* const self,
			Chunk_group_hash const chunk_group_hash,
			std::size_t const chunk_index
		) noexcept
		{
			constexpr bool is_const = std::is_const_v<Self>;
			using Component_chunk_iterator_type = std::conditional_t<is_const, Component_iterator<Component_ts const...>, Component_iterator<Component_ts...>>;
			using Component_chunk_view_type = std::conditional_t<is_const, Component_chunk_view<Component_ts const...>, Component_chunk_view<Component_ts...>>;
			using Chunk_group_type = std::conditional_t<is_const, Chunk_group const, Chunk_group>;
			using Chunk_type = std::conditional_t<is_const, Chunk const, Chunk>;
			using Pointer_type = std::conditional_t<is_const, std::byte const*, std::byte*>;

			auto const chunk_group_location = self->m_chunk_groups.find(chunk_group_hash);
			if (chunk_group_location == self->m_chunk_groups.end()) [[unlikely]]
			{
				return Component_chunk_view_type{};
			}

			Chunk_group_type& chunk_group = chunk_group_location->second;


			if (chunk_index >= chunk_group.chunks.size()) [[unlikely]]
			{
				return Component_chunk_view_type{};
			}

			Chunk_type& chunk = chunk_group.chunks[chunk_index];

			std::array<std::size_t, sizeof...(Component_ts)> const offsets
			{
				self->get_component_element_offset(
					{
						get_component_type_id<Component_ts>(),
						sizeof(Component_ts)
					},
					0
				)...
			};

			std::size_t const number_of_elements_in_chunk = self->number_of_elements(chunk_group.number_of_elements, chunk_index);

			if constexpr (sizeof...(Component_ts) == 1)
			{
				using Component_t = std::tuple_element_t<0, std::tuple<Component_ts...>>;
				
				Component_chunk_iterator_type const begin
				{
					chunk.data() + offsets[0]
				};

				Component_chunk_iterator_type const end
				{
					chunk.data() + offsets[0] + number_of_elements_in_chunk * sizeof(Component_t)
				};

				return Component_chunk_view_type{begin, end};
			}
			else
			{
				auto const create_tuple_aux = [] <typename F> (F&& generator) -> auto
				{
					if constexpr (std::is_const_v<Self>)
					{
						return create_tuple<
							std::conditional_t<std::is_const_v<Component_ts const>, std::byte const*, std::byte*>...
						>(std::forward<F>(generator));
					}
					else
					{
						return create_tuple<
							std::conditional_t<std::is_const_v<Component_ts>, std::byte const*, std::byte*>...
						>(std::forward<F>(generator));
					}
				};

				auto const create_begin_pointer = [&chunk, &offsets] (std::size_t const index) -> Pointer_type
				{
					return chunk.data() + offsets[index];
				};

				auto const begin_pointers = create_tuple_aux(create_begin_pointer);

				Component_chunk_iterator_type const begin
				{
					begin_pointers
				};

				
				constexpr std::array<std::size_t, sizeof...(Component_ts)> component_sizes
				{
					sizeof(Component_ts)...
				};

				auto const create_end_pointer = [&chunk, &offsets, number_of_elements_in_chunk, &component_sizes] (std::size_t const index) -> Pointer_type
				{
					return chunk.data() + offsets[index] + number_of_elements_in_chunk * component_sizes[index];
				};

				auto const end_pointers = create_tuple_aux(create_end_pointer);
				
				Component_chunk_iterator_type const end
				{
					end_pointers
				};


				return Component_chunk_view_type{begin, end};
			}			
		}

		template <typename Self, Concept::Component... Component_ts>
		static auto get_view(
			Self* const self,
			Chunk_group_hash const chunk_group_hash
		) noexcept
		{
			constexpr bool is_const = std::is_const_v<Self>;
			using Component_chunk_group_iterator_type = std::conditional_t<is_const, Component_chunk_iterator<Component_ts const...>, Component_chunk_iterator<Component_ts...>>;
			using Component_chunk_group_view_type = std::conditional_t<is_const, Component_chunk_group_view<Component_ts const...>, Component_chunk_group_view<Component_ts...>>;
			using Chunk_group_type = std::conditional_t<is_const, Chunk_group const, Chunk_group>;

			auto const chunk_group_location = self->m_chunk_groups.find(chunk_group_hash);
			if (chunk_group_location == self->m_chunk_groups.end()) [[unlikely]]
			{
				return Component_chunk_group_view_type{};
			}

			Chunk_group_type& chunk_group = chunk_group_location->second;

			std::array<std::size_t, sizeof...(Component_ts)> const component_offsets
			{
				self->get_component_element_offset(
					{
						get_component_type_id<Component_ts>(),
						sizeof(Component_ts)
					},
					0
				)...
			};

			Component_chunk_group_iterator_type const begin
			{
				chunk_group,
				0,
				self->m_number_of_entities_per_chunk,
				component_offsets
			};

			Component_chunk_group_iterator_type const end
			{
				chunk_group,
				chunk_group.number_of_elements,
				self->m_number_of_entities_per_chunk,
				component_offsets
			};

			return Component_chunk_group_view_type{begin, end};
		}


		template <typename Self, Concept::Component... Component_ts>
		static auto get_view(
			Self* const self
		) noexcept
		{
			constexpr bool is_const = std::is_const_v<Self>;

			using Component_chunk_group_all_iterator_type = std::conditional_t<is_const, Component_chunk_group_iterator<Component_ts const...>, Component_chunk_group_iterator<Component_ts...>>;
			using Component_chunk_group_all_view_type = std::conditional_t<is_const, Component_chunk_group_all_view<Component_ts const...>, Component_chunk_group_all_view<Component_ts...>>;

			auto const chunk_groups_begin = self->m_chunk_groups.begin();
			auto const chunk_groups_end = self->m_chunk_groups.end();

			std::array<std::size_t, sizeof...(Component_ts)> const component_offsets
			{
				self->get_component_element_offset(
					{
						get_component_type_id<Component_ts>(),
						sizeof(Component_ts)
					},
					0
				)...
			};

			Component_chunk_group_all_iterator_type const begin
			{
				chunk_groups_begin,
				chunk_groups_end,
				self->m_number_of_entities_per_chunk,
				component_offsets
			};

			Component_chunk_group_all_iterator_type const end
			{
				chunk_groups_end,
				chunk_groups_end,
				self->m_number_of_entities_per_chunk,
				component_offsets
			};

			return Component_chunk_group_all_view_type{begin, end};
		}

		template <Concept::Component Component_t>
		Component_t get_component_value(Chunk_group const& chunk_group, Index const index) const noexcept
		{
			assert(index < chunk_group.number_of_elements);

			std::size_t const chunk_index = index / m_number_of_entities_per_chunk;
			Chunk const& chunk = chunk_group.chunks[chunk_index];

			Component_type_info const component_type_info{get_component_type_id<Component_t>(), sizeof(Component_t)};
			std::size_t const offset = get_component_element_offset(component_type_info, index);

			Component_t value{};

			assert((offset + component_type_info.size) <= chunk.size());
			std::memcpy(&value, chunk.data() + offset, component_type_info.size);

			return value;
		}

		void copy_components(
			Chunk_group const& from_group,
			std::size_t const from_index,
			Chunk_group& to_group,
			std::size_t const to_index
		) noexcept
		{
			std::size_t const from_chunk_index = from_index / m_number_of_entities_per_chunk;
			Chunk const& from_chunk = from_group.chunks[from_chunk_index];

			std::size_t const to_chunk_index = to_index / m_number_of_entities_per_chunk;
			Chunk& to_chunk = to_group.chunks[to_chunk_index];

			std::size_t total_component_size = 0;

			for (Component_type_info const& type_info : m_component_type_infos)
			{
				std::size_t const source_offset = 
					m_number_of_entities_per_chunk * total_component_size + 
					(from_index % m_number_of_entities_per_chunk) * type_info.size;

				std::size_t const destination_offset = 
					m_number_of_entities_per_chunk * total_component_size + 
					(to_index % m_number_of_entities_per_chunk) * type_info.size;

				assert((source_offset + type_info.size) <= from_chunk.size());
				assert((destination_offset + type_info.size) <= to_chunk.size());
				std::memcpy(to_chunk.data() + destination_offset, from_chunk.data() + source_offset, type_info.size);

				total_component_size += type_info.size;
			}
		}

		std::optional<Component_group_entity_moved> remove_entity(Chunk_group& chunk_group, Index const index) noexcept
		{
			if ((index + 1) == chunk_group.number_of_elements)
			{
				--chunk_group.number_of_elements;

				return std::nullopt;
			}
			else
			{
				std::size_t const entity_to_remove_index = index;
				std::size_t const entity_to_move_index = chunk_group.number_of_elements - 1;

				Entity const entity_to_move = get_component_value<Entity>(chunk_group, entity_to_move_index);

				copy_components(chunk_group, entity_to_move_index, chunk_group, entity_to_remove_index);

				--chunk_group.number_of_elements;

				return Component_group_entity_moved{entity_to_move};
			}
		}

	private:

		std::pmr::unordered_map<Chunk_group_hash, Chunk_group> m_chunk_groups;
		std::pmr::vector<Component_type_info> m_component_type_infos;
		std::size_t m_number_of_entities_per_chunk;
		std::pmr::polymorphic_allocator<std::byte> m_chunk_allocator;

	};
}