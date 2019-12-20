export module maia.ecs.component_group;

import maia.ecs.component;
import maia.ecs.components_chunk;
import maia.ecs.entity;

import <array>;
import <cstddef>;
import <memory_resource>;
import <optional>;
import <span>;
import <vector>;

namespace Maia::ECS
{
	export struct Component_group_entity_index
	{
		std::size_t value;
	};

	export struct Component_group_entity_moved
	{
		Entity entity;
	};

	export struct Component_type_info
	{
		Component_ID id;
		std::size_t offset;
		Component_size size;
	};

	export class Component_group
	{
	public:

		using Index = Component_group_entity_index;
		using Element_moved = Component_group_entity_moved;


		Component_group(
			std::span<Component_info const> component_infos,
			std::size_t capacity_per_chunk,
			std::pmr::polymorphic_allocator<Components_chunk> chunks_allocator = {},
			std::pmr::polymorphic_allocator<Component_type_info> type_infos_allocator = {}
		) noexcept;



		template <typename... Component>
		std::tuple<Component...> back() const noexcept
		{
			return get_components_data<Component...>({ m_size - 1 });
		}



		std::size_t size() const noexcept;

		std::size_t num_chunks() const noexcept;

		void reserve(std::size_t new_capacity) noexcept;

		std::size_t capacity() const noexcept;

		void shrink_to_fit() noexcept;



		std::optional<Element_moved> erase(Index index) noexcept;

		Index push_back() noexcept;

		template <class... Component>
		Index push_back(Component&&... component) noexcept
		{
			Index const index = push_back();
			set_components_data(index, std::forward<Component>(component)...);
			return index;
		}

		void pop_back() noexcept;



		template <typename Component>
		Component_view<Component const> get_component_data(Index const index) const noexcept
		{
			Component_ID const component_id = Component_ID::get<Component>();

			std::byte const* const pointer = get_component_data_impl(component_id, index);
			return {pointer};
		}

		template <typename Component>
		void set_component_data(Index const index, Component&& component) noexcept
		{
			Component_ID const component_id = Component_ID::get<Component>();

			std::byte* const pointer = get_component_data_impl(component_id, index);
			
			Component_view<Component> const component_view{pointer};
			component_view.set(component);
		}


		template <typename... Component>
		std::tuple<Component const...> get_components_data(Index const index) const noexcept
		{
			return std::make_tuple(get_component_data<Component>(index).get()...);
		}

		template <typename... Component>
		void set_components_data(Index const index, Component&&... component) noexcept
		{
			(set_component_data<Component>(index, std::forward<Component>(component)), ...);
		}


		template <typename Component>
		Component_range_view<Component> components(std::size_t const chunk_index) noexcept
		{
			Component_ID const component_id = Component_ID::get<Component>();
			std::size_t const component_offset = get_component_offset(component_id);
			
			std::size_t const num_elements = chunk_index == m_chunks.size() - 1 ?
				m_size - m_capacity_per_chunk * chunk_index :
				m_capacity_per_chunk;

			Components_chunk& chunk = m_chunks[chunk_index];
			
			std::byte* const data = chunk.data() + component_offset;
			return {{data, num_elements*sizeof(Component)}};
		}

		template <typename Component>
		Component_range_view<Component const> components(std::size_t const chunk_index) const noexcept
		{
			Component_ID const component_id = Component_ID::get<Component>();
			std::size_t const component_offset = get_component_offset(component_id);

			std::size_t const num_elements = chunk_index == m_chunks.size() - 1 ?
				m_size - m_capacity_per_chunk * chunk_index :
				m_capacity_per_chunk;

			Components_chunk const& chunk = m_chunks[chunk_index];
			
			std::byte const* const data = chunk.data() + component_offset;
			return {{data, num_elements*sizeof(Component)}};
		}

		
		
	private:


		template <typename T>
		using Remove_cvr_t = std::remove_cv_t<std::remove_reference_t<T>>;



		void increment_size() noexcept;
		void decrement_size() noexcept;



		Components_chunk const& get_entity_chunk(Component_group_entity_index component_group_index) const noexcept;
		Components_chunk& get_entity_chunk(Component_group_entity_index component_group_index) noexcept;



		std::byte const* get_component_data_impl(Component_ID const component_id, Index index) const noexcept;
		std::byte* get_component_data_impl(Component_ID const component_id, Index index) noexcept;

		

		std::size_t calculate_entity_index(Component_group_entity_index component_group_index) const noexcept; // TODO can be static private
		std::size_t get_component_offset(Component_ID const component_id) const noexcept; // TODO can be 
		Component_type_info get_component_type_info(Component_ID const component_id) const noexcept;



		std::size_t m_size;
		std::size_t m_size_of_single_element;
		std::size_t m_capacity_per_chunk;
		std::pmr::vector<Components_chunk> m_chunks;
		std::pmr::vector<Component_type_info> m_component_type_infos;

	};



	export template <typename... Component>
	Component_group make_component_group(std::size_t const capacity_per_chunk) noexcept
	{
		std::array<Component_info, sizeof...(Component)> const component_infos
		{
			Component_info { Component_ID::get<Component>(), { sizeof(Component) } }...
		};

		return { component_infos, capacity_per_chunk };
	}
}
