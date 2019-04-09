#ifndef MAIA_GAMEENGINE_ENTITYHASH_H_INCLUDED
#define MAIA_GAMEENGINE_ENTITYHASH_H_INCLUDED

#include <cstddef>
#include <functional>

#include <Maia/GameEngine/Entity.hpp>

namespace std
{
	template<>
	struct hash<Maia::GameEngine::Entity>
	{
		using argument_type = Maia::GameEngine::Entity;
		using result_type = std::size_t;

		result_type operator()(argument_type const& entity) const noexcept
		{
			return std::hash<Maia::GameEngine::Entity::Integral_type>{}(entity.value);
		}
	};
}

#endif
