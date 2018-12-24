#include <array>
#include <chrono>

#include <winrt/Windows.ApplicationModel.Core.h>
#include <winrt/Windows.UI.Core.h>
#include <winrt/Windows.UI.Composition.h>
#include <winrt/Windows.UI.Input.h>

#include <Maia/GameEngine/Entity_manager.hpp>
#include <Maia/Renderer/D3D12/Utilities/D3D12_utilities.hpp>

#include "Scene.hpp"
#include "Renderer/D3D12/Renderer.hpp"

using namespace winrt;

using namespace Windows;
using namespace Windows::ApplicationModel::Core;
using namespace Windows::Foundation::Numerics;
using namespace Windows::UI;
using namespace Windows::UI::Core;
using namespace Windows::UI::Composition;

struct App : implements<App, IFrameworkViewSource, IFrameworkView>
{
	using clock = std::chrono::steady_clock;

	std::unique_ptr<Maia::Mythology::D3D12::Render_resources> m_render_resources{};
	Maia::Mythology::D3D12::Scene_resources m_scene_resources{};
	std::unique_ptr<Maia::Mythology::D3D12::Renderer> m_renderer{};
	winrt::agile_ref<CoreWindow> m_window{};
	Maia::GameEngine::Entity_manager m_entity_manager{};

	IFrameworkView CreateView()
	{
		return *this;
	}

	void Initialize(CoreApplicationView const &)
	{
	}

	void Load(hstring const&)
	{
		winrt::com_ptr<IDXGIFactory6> factory{ Maia::Renderer::D3D12::create_factory({}) };
		winrt::com_ptr<IDXGIAdapter4> adapter{ Maia::Renderer::D3D12::select_adapter(*factory, false) };
		std::uint8_t const pipeline_length{ 3 };
		m_render_resources = std::make_unique<Maia::Mythology::D3D12::Render_resources>(*adapter, pipeline_length);

		m_scene_resources = Maia::Mythology::load(m_entity_manager, *m_render_resources);

		IUnknown& window = *static_cast<::IUnknown*>(winrt::get_abi(m_window.get()));
		winrt::Windows::Foundation::Rect const bounds = m_window.get().Bounds();
		m_renderer = std::make_unique<Maia::Mythology::D3D12::Renderer>(
			*factory,
			*m_render_resources,
			window,
			Eigen::Vector2i{ static_cast<int>(bounds.Width), static_cast<int>(bounds.Height) }
		);
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

		constexpr clock::duration fixed_update_duration{ 50ms };
		clock::time_point previous_time_point{ clock::now() };
		clock::duration lag{};

		while (true)
		{
			clock::time_point current_time_point{ clock::now() };
			clock::duration delta_time{ current_time_point - previous_time_point };
			previous_time_point = current_time_point;
			lag += delta_time;


			CoreDispatcher dispatcher = window.Dispatcher();
			dispatcher.ProcessEvents(CoreProcessEventsOption::ProcessAllIfPresent);


			while (lag >= fixed_update_duration)
			{
				FixedUpdate(delta_time);
				lag -= fixed_update_duration;
			}

			RenderUpdate(duration<float>{ lag } / fixed_update_duration);
		}
	}

	void SetWindow(CoreWindow window)
	{
		window.PointerPressed({ this, &App::OnPointerPressed });
		window.PointerMoved({ this, &App::OnPointerMoved });

		window.PointerReleased([&](auto && ...)
		{
		});

		window.SizeChanged(
			[&](CoreWindow window, WindowSizeChangedEventArgs const& event_args)
		{
			winrt::Windows::Foundation::Size const size = event_args.Size();
			m_renderer->resize_window({ static_cast<int>(size.Width), static_cast<int>(size.Height) });
		}
		);

		m_window = window;
	}

	void OnPointerPressed(IInspectable const &, PointerEventArgs const & args)
	{
	}

	void OnPointerMoved(IInspectable const &, PointerEventArgs const & args)
	{
	}

private:

	void FixedUpdate(clock::duration delta_time)
	{
	}

	void RenderUpdate(float update_percentage)
	{
		m_renderer->render(m_scene_resources);
		m_renderer->present();
	}

};

int main()
{
	CoreApplication::Run(App{});

	return 0;
}
