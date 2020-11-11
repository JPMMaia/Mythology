export module maia.ecs.shared_component;

import <concepts>;
import <cstdint>;
import <type_traits>;

namespace Maia::ECS
{
	namespace Concept
	{
		export template<typename T>
			concept Shared_component = std::equality_comparable<T> && std::movable<T>;
	}

	export struct Shared_component_type_ID
	{
		std::uint16_t value;
	};

	export inline bool operator==(Shared_component_type_ID const lhs, Shared_component_type_ID const rhs) noexcept
	{
		return lhs.value == rhs.value;
	}

	export inline bool operator!=(Shared_component_type_ID const lhs, Shared_component_type_ID const rhs) noexcept
	{
		return !(lhs == rhs);
	}

	Shared_component_type_ID create_shared_component_type_id() noexcept;

	template <class Component>
	Shared_component_type_ID get_shared_component_type_id_impl() noexcept
	{
		static Shared_component_type_ID id = create_shared_component_type_id();

		return id;
	}

	export template <class Component>
		Shared_component_type_ID get_shared_component_type_id() noexcept
	{
		using Raw_shared_component = typename std::remove_cv_t<typename std::remove_reference_t<Component>>;

		return get_shared_component_type_id_impl<Raw_shared_component>();
	}


	export using Shared_component_size = std::uint16_t;

	export struct Shared_component_type_info
	{
		Shared_component_type_ID id;
		Shared_component_size size;
	};

	export template <class Shared_component>
		Shared_component_type_info make_shared_component_type_info() noexcept
	{
		return
		{
			get_shared_component_type_id<Shared_component>(),
			{ sizeof(Shared_component) }
		};
	}
}
