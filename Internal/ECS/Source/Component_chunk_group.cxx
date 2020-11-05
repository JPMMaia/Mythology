export module maia.ecs.component_chunk_group;

import maia.ecs.component;
import maia.ecs.entity;

import <array>;
import <cstddef>;
import <memory_resource>;
import <optional>;
import <span>;
import <vector>;

namespace Maia::ECS
{
    export template <class Component>
	struct Component_view
	{
		static_assert(!std::is_pointer_v<Component>);

		using Pointer_type = std::conditional_t<
			std::is_const_v<Component>,
			std::byte const*,
			std::byte*
		>;
		using Value_type = std::remove_const_t<Component>;

		Value_type get() const noexcept
		{
			Value_type component{};
			std::memcpy(&component, this->raw_data, sizeof(Component));
			return component;
		}

		void set(Value_type const& component) const noexcept
		{
			std::memcpy(this->raw_data, &component, sizeof(Component));
		}

		Pointer_type raw_data;
	};

	export template <class Component>
	struct Component_range_view
	{
		using Data_type = std::conditional_t<
			std::is_const_v<Component>,
			std::byte const,
			std::byte
		>;
		using Value_type = std::remove_const_t<Component>;

		Component_range_view() noexcept = default;

		Component_range_view(std::span<Data_type> const raw_data) noexcept :
			raw_data{raw_data}
		{
		}

		template <class Other_component>
		Component_range_view(Component_range_view<Other_component> const& other) noexcept :
			raw_data{other.raw_data}
		{
		}

		Value_type get(std::size_t const index) const noexcept
		{
			assert((index + 1) * sizeof(Value_type) <= this->raw_data.size_bytes());

			Value_type component{};
			std::memcpy(&component, this->raw_data.data() + index * sizeof(Value_type), sizeof(Value_type));
			return component;
		}

		void set(std::size_t const index, Value_type const& component) const noexcept
		{
			assert((index + 1) * sizeof(Value_type) <= this->raw_data.size_bytes());

			std::memcpy(this->raw_data.data() + index * sizeof(Value_type), &component, sizeof(Value_type));
		}

		std::size_t size() const
		{
			return this->raw_data.size() / sizeof(Value_type);
		}

		std::span<Data_type> raw_data;
	};

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
		Component_size size;
	};

	export class Component_chunk_group
	{
	public:

		using Index = Component_group_entity_index;
		using Element_moved = Component_group_entity_moved;


		Component_chunk_group(
			std::span<Component_type_info const> component_infos,
			std::size_t capacity_per_chunk,
			std::pmr::polymorphic_allocator<Component_chunk> chunks_allocator,
			std::pmr::polymorphic_allocator<Component_type_info_and_offset> type_infos_allocator
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


        bool has_component_type(Component_type_ID const component_type_id) const noexcept
        {
            return false;
        }



		template <typename Component>
		Component_view<Component const> get_component_data(Index const index) const noexcept
		{
			Component_type_ID const component_id = get_component_type_id<Component>();

			std::byte const* const pointer = get_component_data_impl(component_id, index);
			return {pointer};
		}

		template <typename Component>
		void set_component_data(Index const index, Component&& component) noexcept
		{
			Component_type_ID const component_id = get_component_type_id<Component>();

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
			Component_type_ID const component_id = get_component_type_id<Component>();
			std::size_t const component_offset = get_component_offset(component_id);

			std::size_t const num_elements = chunk_index == m_chunks.size() - 1 ?
				m_size - m_capacity_per_chunk * chunk_index :
				m_capacity_per_chunk;

			Component_chunk& chunk = m_chunks[chunk_index];

			std::byte* const data = chunk.data() + component_offset;
			return {{data, num_elements*sizeof(Component)}};
		}

		template <typename Component>
		Component_range_view<Component const> components(std::size_t const chunk_index) const noexcept
		{
			Component_type_ID const component_id = get_component_type_id<Component>();
			std::size_t const component_offset = get_component_offset(component_id);

			std::size_t const num_elements = chunk_index == m_chunks.size() - 1 ?
				m_size - m_capacity_per_chunk * chunk_index :
				m_capacity_per_chunk;

			Component_chunk const& chunk = m_chunks[chunk_index];

			std::byte const* const data = chunk.data() + component_offset;
			return {{data, num_elements*sizeof(Component)}};
		}



	private:


		template <typename T>
		using Remove_cvr_t = std::remove_cv_t<std::remove_reference_t<T>>;



		void increment_size() noexcept;
		void decrement_size() noexcept;



		Component_chunk const& get_entity_chunk(Component_group_entity_index component_group_index) const noexcept;
		Component_chunk& get_entity_chunk(Component_group_entity_index component_group_index) noexcept;



		std::byte const* get_component_data_impl(Component_type_ID const component_id, Index index) const noexcept;
		std::byte* get_component_data_impl(Component_type_ID const component_id, Index index) noexcept;



		std::size_t calculate_entity_index(Component_group_entity_index component_group_index) const noexcept; // TODO can be static private
		std::size_t get_component_offset(Component_type_ID const component_id) const noexcept; // TODO can be 
		Component_type_info_and_offset get_component_type_info(Component_type_ID const component_id) const noexcept;



		std::size_t m_size;
		std::size_t m_size_of_single_element;
		std::size_t m_capacity_per_chunk;
		std::pmr::vector<std::pmr::vector<std::byte>> m_chunks;
		std::pmr::vector<Component_type_info_and_offset> m_component_type_infos;

	};
}