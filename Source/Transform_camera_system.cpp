#include <Camera.hpp>
#include <Game_clock.hpp>
#include <Game_key.hpp>
#include <Input_state_views.hpp>

#include "Transform_camera_system.hpp"

namespace Maia::Mythology::Systems
{
	namespace
	{
		void move(
			Eigen::Vector3f& position,
			Eigen::Vector3f const& world_right,
			Eigen::Vector3f const& world_forward,
			Maia::Mythology::Input::Input_state_view const& input_state_view,
			Game_clock::duration const delta_time
		)
		{
			using namespace Maia::Mythology::Input;

			Eigen::Vector3f const direction = [&]() -> Eigen::Vector3f
			{
				std::int8_t const x_direction = [&]() -> std::int8_t
				{
					std::int8_t x_direction{ 0 };

					if (input_state_view.is_down(Game_key::Move_right))
						++x_direction;
					if (input_state_view.is_down(Game_key::Move_left))
						--x_direction;

					return x_direction;
				}();

				std::int8_t const z_direction = [&]() -> std::int8_t
				{
					std::int8_t z_direction{ 0 };

					if (input_state_view.is_down(Game_key::Move_forward))
						++z_direction;
					if (input_state_view.is_down(Game_key::Move_back))
						--z_direction;

					return z_direction;
				}();

				Eigen::Vector3f const direction = x_direction * world_right + z_direction * world_forward;
				return direction.normalized();
			}();

			float const speed = 3.0f;
			float const distance = speed * std::chrono::duration<float>{ delta_time }.count();
			position += distance * direction;
		}

		void rotate(
			Eigen::Quaternionf& rotation,
			Eigen::Vector3f const& world_forward,
			Maia::Mythology::Input::Input_state_view const& input_state_view,
			Game_clock::duration const delta_time
		)
		{
			using namespace Maia::Mythology::Input;

			float const speed = 3.0f;
			float const magnitude = speed * std::chrono::duration<float>{ delta_time }.count();

			Eigen::Vector3i const movement_direction = [&]() -> Eigen::Vector3i
			{
				Eigen::Vector2i const delta_mouse_position = input_state_view.delta_mouse_position();
				return { delta_mouse_position(0), delta_mouse_position(1), 0 };
			}();

			Eigen::Vector3f const new_forward_direction = world_forward + magnitude * movement_direction.cast<float>();

			rotation = Eigen::Quaternionf::FromTwoVectors(world_forward, new_forward_direction) * rotation;
		}
	}

	void transform_camera(
		Eigen::Vector3f& position,
		Eigen::Quaternionf& rotation,
		Maia::Mythology::Input::Input_state_view const& input_state_view,
		Game_clock::duration const delta_time
	)
	{
		using namespace Maia::Mythology;
		using namespace Maia::Mythology::Input;

		Eigen::Matrix3f const rotation_matrix = rotation.toRotationMatrix();

		Eigen::Vector3f const right_direction{ rotation_matrix.col(0) };
		Eigen::Vector3f const forward_direction{ rotation_matrix.col(2) };

		move(position, right_direction, forward_direction, input_state_view, delta_time);

		if (input_state_view.is_down(Game_key::Rotate))
		{
			rotate(rotation, forward_direction, input_state_view, delta_time);
		}
	}
}
