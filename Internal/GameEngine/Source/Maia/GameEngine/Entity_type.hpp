#ifndef MAIA_GAMEENGINE_ENTITYTYPE_H_INCLUDED
#define MAIA_GAMEENGINE_ENTITYTYPE_H_INCLUDED

#include <cstddef>
#include <functional>
#include <iosfwd>

namespace Maia::GameEngine
{
	struct Entity_type_id
	{
		std::size_t value;
	};

	inline bool operator==(Entity_type_id lhs, Entity_type_id rhs)
	{
		return lhs.value == rhs.value;
	}
	inline bool operator!=(Entity_type_id lhs, Entity_type_id rhs)
	{
		return !(lhs == rhs);
	}

	std::ostream& operator<<(std::ostream& output_stream, Entity_type_id entity_type_id);
}

namespace std
{
	template<> 
	struct hash<Maia::GameEngine::Entity_type_id>
	{
		using argument_type = Maia::GameEngine::Entity_type_id;
		using result_type = std::size_t;

		result_type operator()(argument_type const& entity_type) const noexcept
		{
			return std::hash<std::size_t>{}(entity_type.value);
		}
	};
}

#endif
