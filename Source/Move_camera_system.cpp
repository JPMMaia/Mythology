#include "Camera.hpp"

#include "Move_camera_system.hpp"

#include <winrt/Windows.System.h>

namespace Maia::Mythology
{
	void move(Eigen::Vector3f& position, Eigen::Vector3f const& world_right, Eigen::Vector3f const& world_forward, Maia::Mythology::Input::Input_state const& input_state, Clock::duration const delta_time)
	{
		using namespace winrt::Windows::System;
		using namespace Maia::Mythology::Input;

		Eigen::Vector3f const direction = [&]() -> Eigen::Vector3f
		{
			std::int8_t const x_direction = [&]() -> std::int8_t
			{
				std::int8_t x_direction{ 0 };

				if (input_state.is_down({ VirtualKey::D }))
					++x_direction;
				if (input_state.is_down({ VirtualKey::A }))
					--x_direction;

				return x_direction;
			}();

			std::int8_t const z_direction = [&]() -> std::int8_t
			{
				std::int8_t z_direction{ 0 };

				if (input_state.is_down({ VirtualKey::W }))
					++z_direction;
				if (input_state.is_down({ VirtualKey::S }))
					--z_direction;

				return z_direction;
			}();

			Eigen::Vector3f const direction = x_direction * world_right + z_direction * world_forward;
			return direction.normalized();
		}();

		float const speed = 1.0f;
		float const distance = speed * std::chrono::duration<float>{ delta_time }.count();
		position += distance * direction;
	}

	// TODO convert to system
	void rotate(Eigen::Quaternionf& rotation, Eigen::Vector3f const& world_forward, Maia::Mythology::Input::Input_state const& input_state, Clock::duration const delta_time)
	{
		using namespace winrt::Windows::System;
		using namespace Maia::Mythology::Input;

		float const speed = 0.5f;
		float const magnitude = speed * std::chrono::duration<float>{ delta_time }.count();

		Eigen::Vector3f const movement_direction = [&]() -> Eigen::Vector3f
		{
			Eigen::Vector2f const delta_mouse_position = input_state.delta_mouse_position();
			return { delta_mouse_position(0), delta_mouse_position(1), 0.0f };
		}();

		Eigen::Vector3f const new_forward_direction = world_forward + magnitude * movement_direction;

		rotation = Eigen::Quaternionf::FromTwoVectors(world_forward, new_forward_direction) * rotation;
	}

	// TODO convert to system
	void update(Maia::Mythology::Camera& camera, Maia::Mythology::Input::Input_state const& input_state, Clock::duration const delta_time)
	{
		using namespace Maia::Mythology::Input;
		using namespace winrt::Windows::System;

		Eigen::Matrix3f const rotation_matrix = camera.rotation.value.toRotationMatrix();

		Eigen::Vector3f const right_direction{ rotation_matrix.col(0) };
		Eigen::Vector3f const forward_direction{ rotation_matrix.col(2) };

		move(camera.position.value, right_direction, forward_direction, input_state, delta_time);

		if (input_state.is_down({ VirtualKey::Z }))
			rotate(camera.rotation.value, forward_direction, input_state, delta_time);
	}
}