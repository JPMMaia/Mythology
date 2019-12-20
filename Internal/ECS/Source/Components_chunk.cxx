export module maia.ecs.components_chunk;

import <cassert>;
import <cstddef>;
import <cstring>;
import <type_traits>;
import <memory_resource>;
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

	export using Components_chunk = std::pmr::vector<std::byte>;

	/*export class Components_chunk
	{
	public:

		template <typename Component>
		using Reference = std::remove_const_t<std::remove_reference_t<Component>>&;

		template <typename Component>
		using Const_reference = std::remove_const_t<std::remove_reference_t<Component>> const&;

		explicit Components_chunk(std::pmr::polymorphic_allocator<std::byte> allocator = {}) noexcept :
			m_data{std::move(allocator)}
		{
		}

		std::byte* data() noexcept;

		std::byte const* data() const noexcept;


		std::size_t size() const noexcept;


		void resize(std::size_t count, std::byte value) noexcept;


		template <typename Component>
		Const_reference<Component> get_component_data(std::size_t component_offset, std::size_t component_index) const noexcept
		{
			auto pointer = m_data.data() + component_offset + component_index * sizeof(Component);
			return reinterpret_cast<Const_reference<Component>>(*pointer);
		}

		template <typename Component>
		Reference<Component> get_component_data(std::size_t component_offset, std::size_t component_index) noexcept
		{
			auto pointer = m_data.data() + component_offset + component_index * sizeof(Component);
			return reinterpret_cast<Reference<Component>>(*pointer);
		}

		template <typename Component>
		void set_component_data(std::size_t component_offset, std::size_t component_index, Component&& component) noexcept
		{
			get_component_data<Component>(component_offset, component_index) = std::forward<Component>(component);
		}


		template <typename Component>
		std::span<Component> components(std::size_t offset, std::ptrdiff_t count) noexcept
		{
			return { reinterpret_cast<Component*>(m_data.data() + offset), static_cast<typename std::span<Component const>::index_type>(count) };
		}

		template <typename Component>
		std::span<Component const> components(std::size_t offset, std::ptrdiff_t count) const noexcept
		{
			return { reinterpret_cast<Component const*>(m_data.data() + offset), static_cast<typename std::span<Component const>::index_type>(count) };
		}


	private:

		std::pmr::vector<std::byte> m_data;

	};*/
}
