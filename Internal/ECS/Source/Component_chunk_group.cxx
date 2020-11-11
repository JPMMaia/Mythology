export module maia.ecs.component_chunk_group;

import maia.ecs.component;
import maia.ecs.entity;
import maia.ecs.shared_component;

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

		using Index = std::size_t;

		Component_chunk_group(
			std::span<Component_type_info const> const component_type_infos,
			Shared_component_type_info const shared_component_type_info,
			std::size_t const number_of_entities_per_chunk,
			std::pmr::polymorphic_allocator<std::byte> const& chunk_allocator,
			std::pmr::polymorphic_allocator<std::byte> const& allocator
		) noexcept
		{
		}

		Index add_entity(Entity const entity)
		{
			return {};
		}

		template <Concept::Shared_component Shared_component_t>
		Index add_entity(Entity const entity, Shared_component_t const& shared_component)
		{
			return {};
		}

		Component_group_entity_moved remove_entity(Index const index) noexcept
		{
			return {};
		}

		Entity get_entity(Index const index) const noexcept
		{
			return {};
		}

		template <Concept::Component Component_t>
		Component_t get_component_value(Index const index) const noexcept
		{
			return {};
		}

		template <Concept::Component Component_t>
		void set_component_value(Index const index, Component_t const& value) noexcept
		{
		}

		void shrink_to_fit() noexcept
		{
		}

		bool has_component_type(Component_type_ID const id) const noexcept
		{
			return {};
		}

		bool has_shared_component_type(Shared_component_type_ID const id) const noexcept
		{
			return {};
		}

		std::size_t number_of_entities() const noexcept
		{
			return {};
		}

		std::size_t number_of_chunks() const noexcept
		{
			return {};
		}

		template <Concept::Shared_component Shared_component_t>
		std::size_t number_of_chunks(Shared_component_t const& shared_component) const noexcept
		{
			return {};
		}
	};
}