#include "Entity.hpp"

#include <functional>
#include <ostream>

namespace Maia::GameEngine
{
	bool operator==(Entity const lhs, Entity const rhs)
	{
		return lhs.value == rhs.value;
	}

	bool operator!=(Entity const lhs, Entity const rhs)
	{
		return !(lhs == rhs);
	}

	std::ostream& operator<<(std::ostream& output_stream, Entity const value)
	{
		output_stream << "{" << value.value << "}";
		return output_stream;
	}
}