export module maia.ecs.component;

import <concepts>;
import <cstdint>;
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


	export using Component_size = std::uint16_t;;

	export struct Component_info
	{
		Component_type_ID id;
		Component_size size;
	};

	export template <class Component>
	Component_info create_component_info() noexcept
	{
		return 
		{
			get_component_type_id<Component>(),
			{ sizeof(Component) }
		};
	}
}
