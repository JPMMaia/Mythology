#include <Camera.hpp>
#include <Game_clock.hpp>
#include <Game_key.hpp>
#include <Input_state_views.hpp>

#include "Transform_freely_system.hpp"

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

			float const speed = 1.0f;
			float const magnitude = speed * std::chrono::duration<float>{ delta_time }.count();

			auto const calculate_direction = [](Maia::Mythology::Input::Input_state_view const& input_state_view, Game_key const positive_key, Game_key const negative_key) -> float
			{
				bool const positive = input_state_view.is_down(positive_key);
				bool const negative = input_state_view.is_down(negative_key);

				if (positive == negative)
					return 0.0f;

				else if (positive)
					return 1.0f;

				else
					return -1.0f;
			};

			float const yaw = calculate_direction(input_state_view, Game_key::Rotate_positive_yaw, Game_key::Rotate_negative_yaw);
			float const pitch = calculate_direction(input_state_view, Game_key::Rotate_positive_pitch, Game_key::Rotate_negative_pitch);
			float const roll = calculate_direction(input_state_view, Game_key::Rotate_positive_roll, Game_key::Rotate_negative_roll);

			if (yaw != 0.0f || pitch != 0.0f || roll != 0.0f)
			{
				Eigen::Quaternionf small_rotation;
				small_rotation =
					Eigen::AngleAxisf{ magnitude * pitch, Eigen::Vector3f { 1.0f, 0.0f, 0.0f } } *
					Eigen::AngleAxisf{ magnitude * yaw, Eigen::Vector3f { 0.0f, 1.0f, 0.0f } } *
					Eigen::AngleAxisf{ magnitude * roll, Eigen::Vector3f { 0.0f, 0.0f, 1.0f } };
				small_rotation.normalize();

				rotation = rotation * small_rotation;
				rotation.normalize();
			}
		}
	}

	void transform_freely(
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
		rotate(rotation, forward_direction, input_state_view, delta_time);
	}
}
