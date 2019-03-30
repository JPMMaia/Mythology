#include <array>
#include <iostream>
#include <filesystem>

#include <Maia/GameEngine/Entity_manager.hpp>
#include <Maia/Utilities/glTF/gltf.hpp>

#include "Camera.hpp"
#include "Game_clock.hpp"
#include "Game_key.hpp"
#include "IInput_system.hpp"
#include "Input_state.hpp"

#include <Maia/GameEngine/Systems/Transform_system.hpp>
#include "Render/D3D12/Render_system.hpp"
#include "Transform_camera_system.hpp"

#include "Application.hpp"

namespace
{
	Maia::Mythology::Scenes_resources create_default_scene()
	{
		Maia::Mythology::Scenes_resources scenes_resources = {};
		scenes_resources.entity_managers.emplace_back();
		scenes_resources.scenes_entities.emplace_back();
		return scenes_resources;
	}

	Maia::Mythology::Scenes_resources load_scenes(Maia::Mythology::D3D12::Load_scene_system& load_scene_system, std::filesystem::path const& gltf_file_path)
	{
		using namespace Maia::Mythology;
		using namespace Maia::Utilities::glTF;

		Maia::Utilities::glTF::Gltf const gltf = D3D12::read_gltf(gltf_file_path);

		D3D12::Scenes_resources scenes_resources =
			load_scene_system.load(gltf);

		std::vector<Maia::GameEngine::Entity_manager> entity_managers;
		std::vector<Maia::Mythology::D3D12::Scene_entities> scenes_entities;
		

		if (gltf.scenes)
		{
			entity_managers.reserve(gltf.scenes->size());
			scenes_entities.reserve(gltf.scenes->size());

			for (Maia::Utilities::glTF::Scene const& scene : *gltf.scenes)
			{
				Maia::GameEngine::Entity_manager entity_manager;

				Maia::Mythology::D3D12::Scene_entities scene_entities =
					D3D12::create_entities(gltf, scene, entity_manager);

				entity_managers.push_back(std::move(entity_manager));
				scenes_entities.push_back(std::move(scene_entities));
			}
		}

		load_scene_system.wait();

		return
		{
			std::move(entity_managers),
			std::move(scenes_entities),
			0,
			std::move(scenes_resources.geometry_resources),
			std::move(scenes_resources.mesh_views)
		};
	}
}

namespace Maia::Mythology
{
	Application::Application(
		std::unique_ptr<Maia::Mythology::D3D12::Load_scene_system> load_scene_system
	) :
		m_load_scene_system{ std::move(load_scene_system) },
		m_scene_being_loaded{},
		m_scenes_resources{ create_default_scene() },
		m_current_scenes_index{ 0 }
	{
		/*m_scene_being_loaded =
			std::async(std::launch::deferred,
				[&]() -> Scenes_resources { return load_scenes(*m_load_scene_system, L"box.gltf"); });

		m_scenes_resources.push_back(m_scene_being_loaded->get());
		m_scene_being_loaded = {};
		m_current_scenes_index = m_scenes_resources.size() - 1;*/
	}

	void Application::run(
		Maia::Mythology::D3D12::Render_system& render_system,
		std::function<bool()> process_events,
		Maia::Mythology::Input::IInput_system& input_system
	)
	{
		using namespace std::chrono;
		using namespace std::chrono_literals;
		using namespace Maia::Mythology::Input;

		constexpr Game_clock::duration fixed_update_duration{ 50ms };
		Game_clock::time_point previous_time_point{ Game_clock::now() };
		Game_clock::duration lag{};

		while (true)
		{
			Game_clock::time_point const current_time_point{ Game_clock::now() };
			Game_clock::duration const delta_time{ current_time_point - previous_time_point };
			previous_time_point = current_time_point;
			lag += delta_time;


			if (!process_events())
				break;

			const Input_state& input_state = input_system.execute();
			handle_input_events(Input_events_view{ input_state });


			while (lag >= fixed_update_duration)
			{
				fixed_update(fixed_update_duration, Input_state_view{ input_state });
				lag -= fixed_update_duration;
			}

			render_update(render_system, duration<float>{ lag } / fixed_update_duration);
		}
	}


	void Application::handle_input_events(Maia::Mythology::Input::Input_events_view input_events_view)
	{
		if (input_events_view.is_pressed(Game_key::Load_scene))
		{
			m_scene_being_loaded =
				std::async(std::launch::async,
					[&]() -> Scenes_resources { return load_scenes(*m_load_scene_system, L"gizmo.gltf"); });
		}
	}

	void Application::fixed_update(Game_clock::duration delta_time, Maia::Mythology::Input::Input_state_view input_state_view)
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

		{
			using namespace Maia::GameEngine;
			using namespace Maia::GameEngine::Components;

			Scenes_resources& scene_resources = 
				m_scenes_resources[m_current_scenes_index];

			Entity_manager& entity_manager =
				scene_resources.entity_managers[scene_resources.current_scene_index];

			Entity const camera_entity = 
				scene_resources
				.scenes_entities[scene_resources.current_scene_index]
				.cameras[0];

			Local_position camera_position = entity_manager.get_component_data<Local_position>(camera_entity);
			Local_rotation camera_rotation = entity_manager.get_component_data<Local_rotation>(camera_entity);

			Systems::transform_camera(camera_position.value, camera_rotation.value, input_state_view, delta_time);

			entity_manager.set_component_data(camera_entity, camera_position);
			entity_manager.set_component_data(camera_entity, camera_rotation);
		}
	}

	void Application::render_update(Maia::Mythology::D3D12::Render_system& render_system, float update_percentage)
	{
		using namespace Maia::GameEngine::Systems;

		{
			Scenes_resources& scenes = m_scenes_resources[m_current_scenes_index];
			Transform_system{}.execute(scenes.entity_managers[scenes.current_scene_index]);
		}

		{
			Scenes_resources const& scenes = m_scenes_resources[m_current_scenes_index];

			gsl::span<Maia::GameEngine::Entity_type_id const> const mesh_entity_types_ids =
				scenes.scenes_entities[scenes.current_scene_index].mesh;

			render_system.render_frame(
				scenes.entity_managers[scenes.current_scene_index],
				scenes.scenes_entities[scenes.current_scene_index].cameras[0],
				mesh_entity_types_ids,
				scenes.mesh_views
			);
		}
	}
}