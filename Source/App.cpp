#include <array>
#include <chrono>
#include <iostream>

#include <winrt/Windows.ApplicationModel.Core.h>
#include <winrt/Windows.UI.Core.h>
#include <winrt/Windows.UI.Composition.h>
#include <winrt/Windows.UI.Input.h>

#include <Maia/GameEngine/Entity_manager.hpp>
#include <Maia/GameEngine/Systems/Transform_system.hpp>
#include <Maia/Utilities/glTF/gltf.hpp>

#include "Camera.hpp"
#include "Input_system.hpp"
#include "Render/D3D12/Load_scene_system.hpp"
#include "Render/D3D12/Render_system.hpp"

using namespace winrt;

using namespace Windows;
using namespace Windows::ApplicationModel::Core;
using namespace Windows::Foundation::Numerics;
using namespace Windows::UI;
using namespace Windows::UI::Core;
using namespace Windows::UI::Composition;

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

	void update_mouse_position(Maia::Mythology::Input::Input_state& input_state, CoreWindow window)
	{
		using namespace Maia::Mythology::Input;
		using namespace winrt::Windows::Foundation;

		{
			Point const pointer_position = window.PointerPosition();
			set(input_state, { pointer_position.X, pointer_position.Y });
		}
	}
}

namespace
{
	struct Scenes_resources
	{
		Maia::GameEngine::Entity_manager entity_manager{};
		std::vector<std::vector<Maia::Mythology::D3D12::Static_entity_type>> entity_types_per_scene{};
		std::size_t current_scene_index{};

		Maia::Mythology::D3D12::Geometry_resources geometry_resources{};
		std::vector<Maia::Mythology::D3D12::Mesh_view> mesh_views{};
		Maia::Mythology::Camera camera{};
	};

	Scenes_resources create_default_scene()
	{
		Scenes_resources scenes_resources;
		scenes_resources.entity_types_per_scene.emplace_back();
		return scenes_resources;
	}

	Scenes_resources load_scenes(Maia::Mythology::D3D12::Load_scene_system& load_scene_system, std::filesystem::path const& gltf_file_path)
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

struct App : implements<App, IFrameworkViewSource, IFrameworkView>
{
	winrt::agile_ref<CoreWindow> m_window{};
	Maia::Mythology::Input::Input_state m_input_state{};
	
	std::unique_ptr<Maia::Mythology::D3D12::Render_system> m_render_system{};

	std::unique_ptr<Maia::Mythology::D3D12::Load_scene_system> m_load_scene_system{};
	std::optional<std::future<Scenes_resources>> m_scene_being_loaded;
	std::vector<Scenes_resources> m_scenes_resources{};
	std::size_t m_current_scenes_index{ 0 };

	IFrameworkView CreateView()
	{
		return *this;
	}

	void Initialize(CoreApplicationView const &)
	{
	}

	void Load(hstring const&)
	{
		const Maia::Mythology::D3D12::Window window = [&]() -> Maia::Mythology::D3D12::Window
		{
			const winrt::Windows::Foundation::Rect bounds = m_window.get().Bounds();

			return
			{
				*static_cast<::IUnknown*>(winrt::get_abi(m_window.get())),
				{ bounds.X, bounds.Y }
			};
		}();


		m_render_system = std::make_unique<Maia::Mythology::D3D12::Render_system>(window);
		m_load_scene_system = std::make_unique<Maia::Mythology::D3D12::Load_scene_system>(m_render_system->d3d12_device());
		m_scenes_resources.push_back(create_default_scene());
	}

	void Uninitialize()
	{
	}

	void Run()
	{
		CoreWindow window = CoreWindow::GetForCurrentThread();
		window.Activate();

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


			CoreDispatcher dispatcher = window.Dispatcher();
			dispatcher.ProcessEvents(CoreProcessEventsOption::ProcessAllIfPresent);
			ProcessInput();


			while (lag >= fixed_update_duration)
			{
				FixedUpdate(fixed_update_duration);
				lag -= fixed_update_duration;
			}

			RenderUpdate(duration<float>{ lag } / fixed_update_duration);
		}
	}

	void SetWindow(CoreWindow window)
	{
		window.KeyDown({ this, &App::OnKeyDown });
		window.KeyUp({ this, &App::OnKeyUp });

		window.SizeChanged(
			[&](CoreWindow window, WindowSizeChangedEventArgs const& event_args)
		{
			winrt::Windows::Foundation::Size const new_size = event_args.Size();

			m_render_system->on_window_resized({ static_cast<int>(new_size.Width), static_cast<int>(new_size.Height) });
		}
		);

		m_window = window;
	}

private:


	void OnKeyDown(CoreWindow const &, KeyEventArgs const & args)
	{
		args.Handled(true);

		Maia::Mythology::Input::set(m_input_state, { args.VirtualKey() }, true);
	}

	void OnKeyUp(CoreWindow const &, KeyEventArgs const & args)
	{
		args.Handled(true);

		Maia::Mythology::Input::set(m_input_state, { args.VirtualKey() }, false);
	}


	void ProcessInput()
	{
		Maia::Mythology::Input::set_previous_state(m_input_state);

		update_mouse_position(m_input_state, m_window.get());

		if (Maia::Mythology::Input::is_pressed(m_input_state, { winrt::Windows::System::VirtualKey::Number1 }))
		{
			m_scene_being_loaded =
				std::async(std::launch::async,
					[&]() -> Scenes_resources { return load_scenes(*m_load_scene_system, L"box.gltf"); });
		}
	}

	void FixedUpdate(Clock::duration delta_time)
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
		update(camera, m_input_state, delta_time);
	}

	void RenderUpdate(float update_percentage)
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

			m_render_system->render_frame(
				scenes.camera,
				scenes.entity_manager,
				entity_types_ids,
				scenes.mesh_views
			);
		}
	}
};

int main()
{
	CoreApplication::Run(App{});

	return 0;
}
