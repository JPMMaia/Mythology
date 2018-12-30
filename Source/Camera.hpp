#ifndef MAIA_MYTHOLOGY_CAMERA_H_INCLUDED
#define MAIA_MYTHOLOGY_CAMERA_H_INCLUDED

#include <Maia/GameEngine/Components/Position.hpp>
#include <Maia/GameEngine/Components/Rotation.hpp>

namespace Maia::Mythology
{
	struct Camera
	{
		Maia::GameEngine::Components::Position position{};
		Maia::GameEngine::Components::Rotation rotation{};
		float vertical_half_angle_of_view{ static_cast<float>(EIGEN_PI) / 3.0f };
		float width_by_height_ratio{ 2.0f };
		Eigen::Vector2f zRange{ 1.0f, 21.0f };
	};
}

#endif
