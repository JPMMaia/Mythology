#include <winrt/Windows.ApplicationModel.Core.h>
#include <winrt/Windows.UI.Core.h>
#include <winrt/Windows.UI.Composition.h>
#include <winrt/Windows.UI.Input.h>

#include "Application.hpp"
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

namespace
{
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

struct App : implements<App, IFrameworkViewSource, IFrameworkView>
{
	winrt::agile_ref<CoreWindow> m_window{};
	Maia::Mythology::Input::Input_state m_input_state{};
	Maia::Mythology::Application m_application{};

	std::unique_ptr<Maia::Mythology::D3D12::Render_system> m_render_system{};

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

		m_application.load(*m_render_system);
	}

	void Uninitialize()
	{
	}

	Maia::Mythology::Input::Input_state const& ProcessEvents()
	{
		Maia::Mythology::Input::set_previous_state(m_input_state);

		CoreWindow window = CoreWindow::GetForCurrentThread();
		
		CoreDispatcher dispatcher = window.Dispatcher();
		dispatcher.ProcessEvents(CoreProcessEventsOption::ProcessAllIfPresent);

		update_mouse_position(m_input_state, m_window.get());

		return m_input_state;
	}

	void Run()
	{
		CoreWindow window = CoreWindow::GetForCurrentThread();
		window.Activate();

		m_application.run( 
			*m_render_system,
			[this]() -> Maia::Mythology::Input::Input_state const& { return ProcessEvents(); }
		);
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
};

int main()
{
	winrt::init_apartment();

	CoreApplication::Run(App{});

	return 0;
}
