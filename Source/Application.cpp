#include <array>
#include <iostream>

#include <Maia/GameEngine/Entity_manager.hpp>
#include <Maia/GameEngine/Systems/Transform_system.hpp>
#include <Maia/Utilities/glTF/gltf.hpp>

#include "Camera.hpp"
#include "Input_system.hpp"
#include "Render/D3D12/Render_system.hpp"

#include "Application.hpp"

using Clock = std::chrono::steady_clock;

namespace
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

				if (is_down(input_state, { VirtualKey::D }))
					++x_direction;
				if (is_down(input_state, { VirtualKey::A }))
					--x_direction;

				return x_direction;
			}();

			std::int8_t const z_direction = [&]() -> std::int8_t
			{
				std::int8_t z_direction{ 0 };

				if (is_down(input_state, { VirtualKey::W }))
					++z_direction;
				if (is_down(input_state, { VirtualKey::S }))
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

	void rotate(Eigen::Quaternionf& rotation, Eigen::Vector3f const& world_forward, Maia::Mythology::Input::Input_state const& input_state, Clock::duration const delta_time)
	{
		using namespace winrt::Windows::System;
		using namespace Maia::Mythology::Input;

		float const speed = 0.5f;
		float const magnitude = speed * std::chrono::duration<float>{ delta_time }.count();

		Eigen::Vector3f const movement_direction = [&]() -> Eigen::Vector3f
		{
			Eigen::Vector2f const delta_mouse_position = get_delta_mouse_position(input_state);
			return { delta_mouse_position(0), delta_mouse_position(1), 0.0f };
		}();

		Eigen::Vector3f const new_forward_direction = world_forward + magnitude * movement_direction;

		rotation = Eigen::Quaternionf::FromTwoVectors(world_forward, new_forward_direction) * rotation;
	}

	void update(Maia::Mythology::Camera& camera, Maia::Mythology::Input::Input_state const& input_state, Clock::duration const delta_time)
	{
		using namespace Maia::Mythology::Input;
		using namespace winrt::Windows::System;

		Eigen::Matrix3f const rotation_matrix = camera.rotation.value.toRotationMatrix();

		Eigen::Vector3f const right_direction{ rotation_matrix.col(0) };
		Eigen::Vector3f const forward_direction{ rotation_matrix.col(2) };

		move(camera.position.value, right_direction, forward_direction, input_state, delta_time);

		if (is_down(input_state, { VirtualKey::Z }))
			rotate(camera.rotation.value, forward_direction, input_state, delta_time);
	}
}

namespace
{
	Maia::Mythology::Scenes_resources create_default_scene()
	{
		Maia::Mythology::Scenes_resources scenes_resources;
		scenes_resources.entity_types_per_scene.emplace_back();
		return scenes_resources;
	}

	Maia::Mythology::Scenes_resources load_scenes(Maia::Mythology::D3D12::Load_scene_system& load_scene_system, std::filesystem::path const& gltf_file_path)
	{
		using namespace Maia::Mythology;

		D3D12::Scenes_resources scenes_resources =
			load_scene_system.load(gltf_file_path);

		Maia::GameEngine::Entity_manager entity_manager{};

		std::vector<std::vector<D3D12::Static_entity_type>> entity_types_per_scene;
		entity_types_per_scene.reserve(scenes_resources.scenes.size());

		for (Maia::Utilities::glTF::Scene const& scene : scenes_resources.scenes)
		{
			entity_types_per_scene.push_back(
				D3D12::create_entities(scene, scenes_resources.nodes, scenes_resources.mesh_views.size(), entity_manager)
			);
		}

		load_scene_system.wait();

		return
		{
			std::move(entity_manager),
			std::move(entity_types_per_scene),
			scenes_resources.current_scene_index,
			std::move(scenes_resources.geometry_resources),
			std::move(scenes_resources.mesh_views),
			Camera{} // TODO camera
		};
	}
}

namespace Maia::Mythology
{
	void Application::load(Maia::Mythology::D3D12::Render_system& render_system)
	{
		m_load_scene_system = std::make_unique<Maia::Mythology::D3D12::Load_scene_system>(render_system.d3d12_device());
		m_scenes_resources.push_back(create_default_scene());
	}

	void Application::run(
		Maia::Mythology::D3D12::Render_system& render_system,
		std::function<Maia::Mythology::Input::Input_state const&()> process_events
	)
	{
		using namespace std::chrono;
		using namespace std::chrono_literals;

		constexpr Clock::duration fixed_update_duration{ 50ms };
		Clock::time_point previous_time_point{ Clock::now() };
		Clock::duration lag{};

		while (true)
		{
			Clock::time_point current_time_point{ Clock::now() };
			Clock::duration delta_time{ current_time_point - previous_time_point };
			previous_time_point = current_time_point;
			lag += delta_time;

			const Input::Input_state& input_state = process_events();
			process_input(input_state);


			while (lag >= fixed_update_duration)
			{
				fixed_update(fixed_update_duration, input_state);
				lag -= fixed_update_duration;
			}

			render_update(render_system, duration<float>{ lag } / fixed_update_duration);
		}
	}

	
	void Application::process_input(Maia::Mythology::Input::Input_state const& input_state)
	{
		if (Maia::Mythology::Input::is_pressed(input_state, { winrt::Windows::System::VirtualKey::L }))
		{
			m_scene_being_loaded =
				std::async(std::launch::async,
					[&]() -> Scenes_resources { return load_scenes(*m_load_scene_system, L"box.gltf"); });
		}
	}

	void Application::fixed_update(Clock::duration delta_time, Maia::Mythology::Input::Input_state const& input_state)
	{
		if (m_scene_being_loaded)
		{
			using namespace std::chrono_literals;

			if (m_scene_being_loaded->wait_for(0s) == std::future_status::ready)
			{
				m_scenes_resources.push_back(m_scene_being_loaded->get());
				m_scene_being_loaded = {};

				m_current_scenes_index = m_scenes_resources.size() - 1;
			}
		}

		auto& camera = m_scenes_resources[m_current_scenes_index].camera;
		update(camera, input_state, delta_time);
	}

	void Application::render_update(Maia::Mythology::D3D12::Render_system& render_system, float update_percentage)
	{
		using namespace Maia::GameEngine::Systems;

		{
			Scenes_resources& scenes = m_scenes_resources[m_current_scenes_index];
			Transform_system{}.execute(scenes.entity_manager);
		}

		{
			Scenes_resources const& scenes = m_scenes_resources[m_current_scenes_index];

			// TODO move this elsewhere
			std::vector<Maia::GameEngine::Entity_type_id> entity_types_ids;
			{
				gsl::span<Maia::Mythology::D3D12::Static_entity_type const> const entity_types =
					scenes.entity_types_per_scene[scenes.current_scene_index];

				entity_types_ids.reserve(entity_types.size());

				std::transform(entity_types.begin(), entity_types.end(), std::back_inserter(entity_types_ids),
					[](Maia::Mythology::D3D12::Static_entity_type entity_type) -> Maia::GameEngine::Entity_type_id { return entity_type.id; });
			}

			render_system.render_frame(
				scenes.camera,
				scenes.entity_manager,
				entity_types_ids,
				scenes.mesh_views
			);
		}
	}
}
