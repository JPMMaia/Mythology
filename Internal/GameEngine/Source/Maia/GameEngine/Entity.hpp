#ifndef MAIA_GAMEENGINE_ENTITY_H_INCLUDED
#define MAIA_GAMEENGINE_ENTITY_H_INCLUDED

#include <cstdint>
#include <iosfwd>

#include <Maia/GameEngine/Component.hpp>

namespace Maia::GameEngine
{
	struct Entity
	{
		using Integral_type = std::uint32_t;

		Integral_type value{ 0 };
	};

	bool operator==(Entity lhs, Entity rhs);

	bool operator!=(Entity lhs, Entity rhs);

	std::ostream& operator<<(std::ostream& output_stream, Entity value);
}

#endif
