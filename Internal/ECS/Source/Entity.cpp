module maia.ecs.entity;

import <functional>;
import <ostream>;

namespace Maia::ECS
{
	bool operator==(Entity const lhs, Entity const rhs) noexcept
	{
		return lhs.value == rhs.value;
	}

	bool operator!=(Entity const lhs, Entity const rhs) noexcept
	{
		return !(lhs == rhs);
	}

	std::ostream& operator<<(std::ostream& output_stream, Entity const value) noexcept
	{
		output_stream << "{" << value.value << "}";
		return output_stream;
	}
}