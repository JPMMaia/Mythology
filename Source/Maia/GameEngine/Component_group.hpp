#ifndef MAIA_GAMEENGINE_COMPONENTGROUP_H_INCLUDED
#define MAIA_GAMEENGINE_COMPONENTGROUP_H_INCLUDED

#include <cstddef>
#include <vector>

#include <gsl/span>

#include <Maia/GameEngine/Entity.hpp>

#include <Maia/GameEngine/Components_chunk.hpp>

namespace std
{
	template< class T >
	class optional;
}

namespace gsl
{
	template <class ElementType, std::ptrdiff_t Extent>
	class span;
}

namespace Maia::GameEngine
{
	class Components_chunk;

	struct Component_group_entity_index
	{
		std::size_t value;
	};

	struct Component_group_entity_moved
	{
		Entity entity;
	};

	struct Component_type_info
	{
		Component_ID id;
		std::size_t offset;
		Component_size size;
	};

	class Component_group
	{
	public:

		using Index = Component_group_entity_index;
		using Element_moved = Component_group_entity_moved;


		Component_group(
			gsl::span<Component_info const> component_infos,
			std::size_t capacity_per_chunk
		);



		template <typename... Component>
		std::tuple<Component...> back() const
		{
			return get_components_data<Component...>({ m_size - 1 });
		}



		std::size_t size() const;

		std::size_t num_chunks() const;

		void reserve(std::size_t new_capacity);

		std::size_t capacity() const;

		void shrink_to_fit();



		std::optional<Element_moved> erase(Index index);

		Index push_back();

		template <class... Component>
		Index push_back(Component&&... component)
		{
			Index const index = push_back();
			set_components_data(index, std::forward<Component>(component)...);
			return index;
		}

		void pop_back();



		template <typename Component>
		Component get_component_data(Index index) const
		{
			Component_ID const component_id = Component_ID::get<Component>();

			std::byte const* const pointer = get_component_data_impl(component_id, index);
			return *reinterpret_cast<Remove_cvr_t<Component> const*>(pointer);
		}

		template <typename Component>
		void set_component_data(Index index, Component&& component)
		{
			Component_ID const component_id = Component_ID::get<Component>();

			std::byte* const pointer = get_component_data_impl(component_id, index);
			*reinterpret_cast<Remove_cvr_t<Component>*>(pointer) = std::forward<Component>(component);
		}


		template <typename... Component>
		std::tuple<Component...> get_components_data(Index index) const
		{
			return std::make_tuple(get_component_data<Component>(index)...);
		}

		template <typename... Component>
		void set_components_data(Index index, Component&&... component)
		{
			(set_component_data<Component>(index, std::forward<Component>(component)), ...);
		}


		template <typename Component>
		gsl::span<Component> components(std::size_t chunk_index)
		{
			Component_ID const component_id = Component_ID::get<Component>();
			std::size_t const component_offset = get_component_offset(component_id);
			
			std::size_t const num_elements = chunk_index == m_chunks.size() - 1 ?
				m_size - m_capacity_per_chunk * chunk_index :
				m_capacity_per_chunk;

			return m_chunks[chunk_index].components<Component>(component_offset, num_elements);
		}

		template <typename Component>
		gsl::span<Component const> components(std::size_t chunk_index) const
		{
			Component_ID const component_id = Component_ID::get<Component>();
			std::size_t const component_offset = get_component_offset(component_id);

			std::size_t const num_elements = chunk_index == m_chunks.size() - 1 ?
				m_size - m_capacity_per_chunk * chunk_index :
				m_capacity_per_chunk;

			return m_chunks[chunk_index].components<Component>(component_offset, num_elements);
		}

		
		
	private:


		template <typename T>
		using Remove_cvr_t = std::remove_cv_t<std::remove_reference_t<T>>;



		void increment_size();
		void decrement_size();



		Components_chunk const& get_entity_chunk(Component_group_entity_index component_group_index) const;
		Components_chunk& get_entity_chunk(Component_group_entity_index component_group_index);



		std::byte const* get_component_data_impl(Component_ID const component_id, Index index) const;
		std::byte* get_component_data_impl(Component_ID const component_id, Index index);

		

		std::size_t calculate_entity_index(Component_group_entity_index component_group_index) const; // TODO can be static private
		std::size_t get_component_offset(Component_ID const component_id) const; // TODO can be 
		Component_type_info get_component_type_info(Component_ID const component_id) const;



		std::size_t m_size;
		std::size_t m_size_of_single_element;
		std::size_t m_capacity_per_chunk;
		std::vector<Components_chunk> m_chunks;
		std::vector<Component_type_info> m_component_type_infos;

	};



	template <typename... Component>
	Component_group make_component_group(std::size_t capacity_per_chunk)
	{
		std::array<Component_info, sizeof...(Component)> component_infos
		{
			Component_info { Component_ID::get<Component>(), { sizeof(Component) } }...
		};

		return { component_infos, capacity_per_chunk };
	}
}

#endif
