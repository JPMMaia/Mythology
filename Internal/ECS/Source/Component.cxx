export module maia.ecs.component;

import <algorithm>;
import <array>;
import <compare>;
import <concepts>;
import <cstdint>;
import <iterator>;
import <type_traits>;

namespace Maia::ECS
{
	namespace Concept
    {
        export template<typename T>
        concept Component = std::regular<T>;
    }

	export struct Component_type_ID
	{
		std::uint16_t value;

		constexpr auto operator<=>(Component_type_ID const rhs) const
		{
			return this->value <=> rhs.value;
		}
	};

	export inline bool operator==(Component_type_ID const lhs, Component_type_ID const rhs) noexcept
	{
		return lhs.value == rhs.value;
	}

	export inline bool operator!=(Component_type_ID const lhs, Component_type_ID const rhs) noexcept
	{
		return !(lhs == rhs);
	}

	Component_type_ID create_component_type_id() noexcept;

	template <class Component>
	Component_type_ID get_component_type_id_impl() noexcept
	{
		static Component_type_ID id = create_component_type_id();

		return id;
	}

	export template <class Component>
	Component_type_ID get_component_type_id() noexcept
	{
		using Raw_component = typename std::remove_cv_t<typename std::remove_reference_t<Component>>;

		return get_component_type_id_impl<Raw_component>();
	}

	
	export template<Concept::Component... Component_t>
	std::array<Component_type_ID, sizeof...(Component_t)> make_component_type_id_array() noexcept
	{
		return
		{
			(get_component_type_id<Component_t>(), ...)
		};
	}


	export using Component_type_size = std::uint16_t;

	export struct Component_type_info
	{
		Component_type_ID id;
		Component_type_size size;
	};

	export template <Concept::Component Component_t>
	Component_type_info create_component_type_info() noexcept
	{
		Component_type_info const type_info
		{
			get_component_type_id<Component_t>(),
			{ sizeof(Component_t) }
		};

		return type_info;
	}

	export template<typename I, typename S>
	void sort_component_type_infos(I const first, S const last) noexcept
	{
		auto const by_type_id = [] (Component_type_info const lhs, Component_type_info const rhs) -> bool
		{
			return lhs.id < rhs.id;
		};

		std::sort(first, last, by_type_id);
	}

	export template<Concept::Component... Component_t>
	std::array<Component_type_info, sizeof...(Component_t)> make_sorted_component_type_info_array() noexcept
	{
		std::array<Component_type_info, sizeof...(Component_t)> type_infos
		{
			{create_component_type_info<Component_t>()...}
		};

		sort_component_type_infos(type_infos.begin(), type_infos.end());

		return type_infos;
	}
}
