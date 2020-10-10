export module maia.ecs.component_group_mask;

import maia.ecs.component;

import <cstdint>;
//import <bitset>;
import <iosfwd>;
import <type_traits>;

namespace Maia::ECS
{
    export struct Component_group_mask
	{
	private:

		template <typename T>
		using Remove_cvr_t = std::remove_cv_t<std::remove_reference_t<T>>;

	public:

		//using Mask = std::bitset<64>;
		using Mask = std::uint64_t;

		Mask value;

		template <typename... Component>
		bool contains() const noexcept
		{
			auto const set_bit = [](Mask& mask, std::size_t const index) -> void
			{
				mask |= (1 << index);
			};

			Mask include_mask = {};
			//(include_mask.set(get_component_type_id<Component>().value), ...);
			( set_bit(include_mask, get_component_type_id<Component>().value), ...);

			return (value & include_mask) == include_mask;
		}
	};


	export bool operator==(Component_group_mask lhs, Component_group_mask rhs) noexcept;
	
	export bool operator!=(Component_group_mask lhs, Component_group_mask rhs) noexcept;


	export std::ostream& operator<<(std::ostream& output_stream, Component_group_mask value) noexcept;



	export template <typename... Components>
	Component_group_mask make_component_group_mask() noexcept
	{
		auto const set_bit = [](Component_group_mask::Mask& mask, std::size_t const index) -> void
		{
			mask |= (1 << index);
		};

		Component_group_mask component_types_group;

		//(component_types_group.value.set(get_component_type_id<Components>().value), ...);
		( set_bit(component_types_group, get_component_type_id<Components>().value), ...);

		return component_types_group;
	}
}
