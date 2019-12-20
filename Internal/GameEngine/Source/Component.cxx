export module maia.ecs.component;

import <cstdint>;
import <type_traits>;

namespace Maia::ECS
{
	export struct Component_ID
	{
		std::uint16_t value;

	private:

		static Component_ID create_component_id() noexcept;

		template <class Component>
		static Component_ID get_impl() noexcept
		{
			static Component_ID id = create_component_id();

			return id;
		}

	public:

		template <class Component>
		static Component_ID get() noexcept
		{
			using Raw_component = typename std::remove_cv_t<typename std::remove_reference_t<Component>>;

			return get_impl<Raw_component>();
		}
	};

	export inline bool operator==(Component_ID const lhs, Component_ID const rhs) noexcept
	{
		return lhs.value == rhs.value;
	}

	export inline bool operator!=(Component_ID const lhs, Component_ID const rhs) noexcept
	{
		return !(lhs == rhs);
	}


	export struct Component_size
	{
		std::uint16_t value;
	};


	export struct Component_info
	{
		Component_ID id;
		Component_size size;
	};

	export template <class Component>
	Component_info create_component_info() noexcept
	{
		return 
		{
			Component_ID::get<Component>(),
			{ sizeof(Component) }
		};
	}
}
