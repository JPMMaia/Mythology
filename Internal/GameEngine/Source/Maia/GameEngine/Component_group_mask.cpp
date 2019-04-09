#include "Component_group_mask.hpp"

#include <ostream>

namespace Maia::GameEngine
{
	bool operator==(Component_group_mask const lhs, Component_group_mask const rhs)
	{
		return lhs.value == rhs.value;
	}
	
	bool operator!=(Component_group_mask const lhs, Component_group_mask const rhs)
	{
		return !(lhs == rhs);
	}

	
	std::ostream& operator<<(std::ostream& output_stream, Component_group_mask const value)
	{
		output_stream << value.value;

		return output_stream;
	}
}
