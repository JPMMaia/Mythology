#ifndef MAIA_UTILITIES_MEMORYCHUNK_H_INCLUDED
#define MAIA_UTILITIES_MEMORYCHUNK_H_INCLUDED

#include <array>
#include <cassert>
#include <cstdint>
#include <tuple>
#include <typeindex>

namespace Maia::Utilities
{
	template <std::size_t Index, class Type_to_find, class Current_type, class... Rest>
	constexpr std::size_t get_first_type_index()
	{
		if constexpr (std::is_same_v<Type_to_find, Current_type>)
		{
			return Index;
		}
		else
		{
			return get_first_type_index<Index + 1, Type_to_find, Rest...>();
		}
	}

	template <class Type_to_find, class... Types>
	constexpr std::size_t get_first_type_index()
	{
		return get_first_type_index<0, Type_to_find, Types...>();
	}

	struct Memory_chunk_index
	{
		std::size_t value;
	};
	
	template <std::size_t Capacity, typename ...Components> // TODO enable_if for each Component in Components: Component is unique.
	class Memory_chunk
	{
	public:

		using Index = Memory_chunk_index;

		Memory_chunk() :
			m_components{},
			m_size{ 0 }
		{
		}

		Index push_back(Components... components)
		{
			assert(size() < capacity());

			constexpr auto number_of_component_types = Memory_chunk::number_of_component_types();
			set_components_data_impl({ m_size }, components..., std::make_index_sequence<number_of_component_types>{});

			return { m_size++ };
		}

		std::tuple<Components...> pop_back()
		{
			assert(size() > 0);

			constexpr auto number_of_component_types = Memory_chunk::number_of_component_types();
			return get_components_data_impl({ --m_size }, std::make_index_sequence<number_of_component_types>{});
		}

		std::tuple<Components...> get_components_data(Index index) const
		{
			assert(index.value < size());

			constexpr auto number_of_component_types = Memory_chunk::number_of_component_types();
			return get_components_data_impl(index, std::make_index_sequence<number_of_component_types>{});
		}

		void set_components_data(Index index, Components... components)
		{
			assert(index.value < size());

			constexpr auto number_of_component_types = Memory_chunk::number_of_component_types();
			set_components_data_impl(index, components..., std::make_index_sequence<number_of_component_types>{});
		}

		template <class Component> // TODO enable_if Component is an element of Components
		Component get_component_data(Index index) const
		{
			constexpr std::size_t array_type_index = get_first_type_index<Component, Components...>();

			return std::get<array_type_index>(m_components)[index.value];
		}

		template <class Component> // TODO enable_if Component is an element of Components
		void set_component_data(Index index, Component&& component)
		{
			constexpr std::size_t array_type_index = get_first_type_index<
				std::remove_reference_t<Component>, 
				Components...
			>();

			std::get<array_type_index>(m_components)[index.value] = std::forward<Component>(component);
		}

		static constexpr std::size_t capacity()
		{
			return Capacity;
		}

		constexpr std::size_t size() const
		{
			return m_size;
		}

	private:

		static constexpr std::size_t number_of_component_types()
		{
			return sizeof...(Components);
		}

		template <std::size_t... I>
		constexpr std::tuple<Components...> get_components_data_impl(Index index, std::index_sequence<I...>) const
		{
			return { std::get<I>(m_components)[index.value]... };
		}

		template <std::size_t... I>
		constexpr void set_components_data_impl(Index index, Components... components, std::index_sequence<I...>)
		{
			((std::get<I>(m_components)[index.value] = components), ...);
		}

		std::uint16_t m_size;
		std::tuple<std::array<Components, Memory_chunk::capacity()>...> m_components;
	};
}

#endif
