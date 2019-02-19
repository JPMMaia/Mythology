#ifndef MAIA_MYTHOLOGY_APPLICATION_H_INCLUDED
#define MAIA_MYTHOLOGY_APPLICATION_H_INCLUDED

#include <cstddef>
#include <functional>
#include <future>
#include <memory>
#include <optional>
#include <vector>

#include <Maia/GameEngine/Entity_manager.hpp>

#include <Game_clock.hpp>
#include <Input_state_views.hpp>

#include "Render/D3D12/Load_scene_system.hpp"

namespace Maia::Mythology
{
	namespace D3D12
	{
		class Render_system;
	}

	namespace Input
	{
		class IInput_system;
	}

	struct Scenes_resources
	{
		Maia::GameEngine::Entity_manager entity_manager{};
		std::vector<std::vector<Maia::Mythology::D3D12::Static_entity_type>> entity_types_per_scene{};
		std::size_t current_scene_index{};

		Maia::Mythology::D3D12::Geometry_resources geometry_resources{};
		std::vector<Maia::Mythology::D3D12::Mesh_view> mesh_views{};
		Maia::Mythology::Camera camera{};
	};

	class Application
	{
	public:

		explicit Application(
			std::unique_ptr<Maia::Mythology::D3D12::Load_scene_system> load_scene_system
		);

		// TODO pass all other systems that are platform specific through the application constructor or through here
		void run(
			Maia::Mythology::D3D12::Render_system& render_system,
			std::function<bool()> process_events,
			Maia::Mythology::Input::IInput_system& input_system
		);


	private:

		
		void handle_input_events(Maia::Mythology::Input::Input_events_view input_events_view);
		void fixed_update(Game_clock::duration delta_time, Maia::Mythology::Input::Input_state_view input_state_view);
		void render_update(Maia::Mythology::D3D12::Render_system& render_system, float update_percentage);

		std::unique_ptr<Maia::Mythology::D3D12::Load_scene_system> m_load_scene_system;
		std::optional<std::future<Scenes_resources>> m_scene_being_loaded;
		std::vector<Scenes_resources> m_scenes_resources;
		std::size_t m_current_scenes_index;

	};

}

#endif