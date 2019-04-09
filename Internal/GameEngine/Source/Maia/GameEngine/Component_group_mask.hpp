#ifndef MAIA_GAMEENGINE_COMPONENTTYPESGROUP_H_INCLUDED
#define MAIA_GAMEENGINE_COMPONENTTYPESGROUP_H_INCLUDED

#include <bitset>
#include <iosfwd>

namespace Maia::GameEngine
{
    struct Component_group_mask
	{
	private:

		template <typename T>
		using Remove_cvr_t = std::remove_cv_t<std::remove_reference_t<T>>;

	public:

		using Mask = std::bitset<64>;

		Mask value;

		template <typename... Component>
		bool contains() const
		{
			Mask include_mask;
			(include_mask.set(Component_ID::get<Component>().value), ...);

			return (value & include_mask) == include_mask;
		}
	};


	bool operator==(Component_group_mask lhs, Component_group_mask rhs);
	
	bool operator!=(Component_group_mask lhs, Component_group_mask rhs);


	std::ostream& operator<<(std::ostream& output_stream, Component_group_mask value);



	template <typename... Components>
	Component_group_mask make_component_group_mask()
	{
		Component_group_mask component_types_group;

		(component_types_group.value.set(Component_ID::get<Components>().value), ...);

		return component_types_group;
	}
}

#endif