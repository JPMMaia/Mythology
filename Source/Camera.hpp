#ifndef MAIA_MYTHOLOGY_CAMERA_H_INCLUDED
#define MAIA_MYTHOLOGY_CAMERA_H_INCLUDED

#include <Maia/GameEngine/Components/Local_position.hpp>
#include <Maia/GameEngine/Components/Local_rotation.hpp>

namespace Maia::Mythology
{
	struct Camera
	{
		Maia::GameEngine::Components::Local_position position{};
		Maia::GameEngine::Components::Local_rotation rotation{};
	};
}

#endif
