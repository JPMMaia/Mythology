module maia.ecs.entity_type;

import <ostream>;

namespace Maia::ECS
{
	std::ostream& operator<<(std::ostream& output_stream, Entity_type_id const entity_type_id) noexcept
	{
		output_stream << entity_type_id.value;

		return output_stream;
	}
}
