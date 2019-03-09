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
		float vertical_half_angle_of_view{ static_cast<float>(EIGEN_PI) / 3.0f };
		float width_by_height_ratio{ 2.0f };
		Eigen::Vector2f z_range{ 1.0f, 21.0f };
	};
}

#endif
