module;

#include <functional>
#include <ostream>

module maia.ecs.entity;

namespace Maia::ECS
{
	bool operator==(Entity const lhs, Entity const rhs) noexcept
	{
		return lhs.index == rhs.index;
	}

	bool operator!=(Entity const lhs, Entity const rhs) noexcept
	{
		return !(lhs == rhs);
	}

	std::ostream& operator<<(std::ostream& output_stream, Entity const value) noexcept
	{
		output_stream << '{' << value.index << '}';
		return output_stream;
	}
}