export module maia.ecs.shared_component;

import <cstdint>;
import <type_traits>;

namespace Maia::ECS
{
	export struct Shared_component_ID
	{
		std::uint16_t value;

	private:

		static Shared_component_ID create_shared_component_id() noexcept;

		template <class Component>
		static Shared_component_ID get_impl() noexcept
		{
			static Shared_component_ID id = create_shared_component_id();

			return id;
		}

	public:

		template <class Component>
		static Shared_component_ID get() noexcept
		{
			using Raw_shared_component = typename std::remove_cv_t<typename std::remove_reference_t<Component>>;

			return get_impl<Raw_shared_component>();
		}
	};

	export inline bool operator==(Shared_component_ID const lhs, Shared_component_ID const rhs) noexcept
	{
		return lhs.value == rhs.value;
	}

	export inline bool operator!=(Shared_component_ID const lhs, Shared_component_ID const rhs) noexcept
	{
		return !(lhs == rhs);
	}


	export using Shared_component_size = std::uint16_t;

	export struct Shared_component_info
	{
		Shared_component_ID id;
		Shared_component_size size;
	};

	export template <class Shared_component>
	Shared_component_info create_shared_component_info() noexcept
	{
		return 
		{
			Shared_component_ID::get<Shared_component>(),
			{ sizeof(Shared_component) }
		};
	}
}
