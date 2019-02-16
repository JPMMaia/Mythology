#include "Win32/Win32_window.hpp"
#include "Application.hpp"
#include "Input_system.hpp"

namespace
{
	LRESULT CALLBACK main_window_process(HWND window_handle, UINT message, WPARAM w_param, LPARAM l_param)
	{
		switch (message)
		{
		case WM_DESTROY:
			PostQuitMessage(0);
			return 0;

		case WM_CLOSE:
			PostQuitMessage(0);
			return 0;

		default:
			// TODO give to default window param
			return DefWindowProc(window_handle, message, w_param, l_param);
		}
	}

	bool process_all_pending_events()
	{
		MSG message{};

		while (PeekMessage(&message, nullptr, 0, 0, PM_REMOVE) && message.message != WM_QUIT)
		{
		}

		return message.message != WM_QUIT;
	}

	struct App
	{
		Maia::Mythology::Win32::Window m_window{ main_window_process, "Mythology_win32_app", "Mythology" };
		Maia::Mythology::Input::Input_state m_input_state{};
		Maia::Mythology::Application m_application{};

		std::unique_ptr<Maia::Mythology::D3D12::Render_system> m_render_system{};

		App()
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
			// TODO
			m_render_system = std::make_unique<Maia::Mythology::D3D12::Render_system>(window);

			m_application.load(*m_render_system);
		}

		Maia::Mythology::Input::Input_state const& process_input()
		{
			// TODO
			return m_input_state;
		}

		void run()
		{
			m_application.run(
				*m_render_system, 
				process_all_pending_events, 
				[&]() -> Maia::Mythology::Input::Input_state const& { return process_input(); }
			);
		}
	};
}

int main()
{
	App app;
	app.run();

	return 0;
}