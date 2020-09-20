module maia.ecs.component_group_mask;

import <ostream>;

namespace Maia::ECS
{
	bool operator==(Component_group_mask const lhs, Component_group_mask const rhs) noexcept
	{
		return lhs.value == rhs.value;
	}
	
	bool operator!=(Component_group_mask const lhs, Component_group_mask const rhs) noexcept
	{
		return !(lhs == rhs);
	}

	
	std::ostream& operator<<(std::ostream& output_stream, Component_group_mask const value) noexcept
	{
		//output_stream << value.value;

		return output_stream;
	}
}
