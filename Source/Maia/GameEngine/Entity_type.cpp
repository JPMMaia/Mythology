#include "Entity_type.hpp"

#include <ostream>

namespace Maia::GameEngine
{
	std::ostream& operator<<(std::ostream& output_stream, Entity_type_id const entity_type_id)
	{
		output_stream << entity_type_id.value;

		return output_stream;
	}
}
