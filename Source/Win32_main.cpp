#include <iostream>

#include "Win32/Window.hpp"
#include "Application.hpp"
#include "Win32/Input_system.hpp"
#include "Render/D3D12/Render_system.hpp"
#include "Game_key.hpp"

#include <Maia/Renderer/D3D12/Utilities/Check_hresult.hpp>
#include <Maia/Renderer/D3D12/Utilities/D3D12_utilities.hpp>

using namespace Maia::Renderer::D3D12;

namespace
{
	winrt::com_ptr<IDXGIAdapter3> select_adapter(IDXGIFactory4& factory)
	{
		winrt::com_ptr<IDXGIAdapter3> adapter = Maia::Renderer::D3D12::select_adapter(factory, false);

		{
			DXGI_ADAPTER_DESC description;
			check_hresult(
				adapter->GetDesc(&description));

			std::wcout << std::wstring_view{ description.Description } << '\n';
		}

		return adapter;
	}

	struct Render_resources
	{
		winrt::com_ptr<IDXGIFactory4> factory;
		winrt::com_ptr<IDXGIAdapter3> adapter;
		winrt::com_ptr<ID3D12Device> device;
		winrt::com_ptr<ID3D12CommandQueue> copy_command_queue;
		winrt::com_ptr<ID3D12CommandQueue> direct_command_queue;

		Render_resources() :
			factory{ Maia::Renderer::D3D12::create_factory({}) },
			adapter{ select_adapter(*factory) },
			device{ create_device(*adapter, D3D_FEATURE_LEVEL_11_0) },
			copy_command_queue{ create_command_queue(*device, D3D12_COMMAND_LIST_TYPE_COPY, 0, D3D12_COMMAND_QUEUE_FLAG_NONE, 0) },
			direct_command_queue{ create_command_queue(*device, D3D12_COMMAND_LIST_TYPE_DIRECT, 0, D3D12_COMMAND_QUEUE_FLAG_NONE, 0) }
		{
		}
	};

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

	// Convert to system
	bool process_all_pending_events()
	{
		MSG message{};

		//while (PeekMessage(&message, nullptr, 0, 0, PM_REMOVE) != 0)
		if (PeekMessage(&message, nullptr, 0, 0, PM_REMOVE) != 0)
		{
			TranslateMessage(&message);
			DispatchMessage(&message);
		}

		return message.message != WM_QUIT;
	}

	Maia::Mythology::Win32::Input_system create_input_system(
		HINSTANCE const instance,
		HWND const window_handle
	)
	{
		using namespace Maia::Mythology;

		std::array<std::uint8_t, 256> keys_map{};
		keys_map[DIK_A] = static_cast<std::uint8_t>(Game_key::Move_left);
		keys_map[DIK_W] = static_cast<std::uint8_t>(Game_key::Move_forward);
		keys_map[DIK_D] = static_cast<std::uint8_t>(Game_key::Move_right);
		keys_map[DIK_S] = static_cast<std::uint8_t>(Game_key::Move_back);
		keys_map[DIK_L] = static_cast<std::uint8_t>(Game_key::Load_scene);

		return Maia::Mythology::Win32::Input_system{ instance, window_handle, keys_map };
	}

	winrt::com_ptr<IDXGISwapChain3> create_swap_chain(
		Render_resources const& render_resources, 
		Maia::Mythology::Win32::Window const& window,
		UINT const buffer_count,
		bool const vertical_sync
	)
	{
		using namespace Maia::Mythology::Win32;
		using namespace Maia::Renderer;

		Window::Dimensions const dimensions = window.dimensions();
		DXGI_FORMAT const format = DXGI_FORMAT_R8G8B8A8_UNORM;

		DXGI_RATIONAL const refresh_rate = [&]() -> DXGI_RATIONAL
		{
			if (vertical_sync)
			{
				return D3D12::find_refresh_rate(
					*render_resources.adapter,
					0,
					format,
					{ static_cast<UINT>(dimensions.width), static_cast<UINT>(dimensions.height) }
				);
			}
			else
			{
				return {};
			}
		}();

		DXGI_MODE_DESC1 const display_mode = [&]() -> DXGI_MODE_DESC1
		{
			DXGI_MODE_DESC1 display_mode{};
			display_mode.Width = dimensions.width;
			display_mode.Height = dimensions.height;
			display_mode.RefreshRate = refresh_rate;
			display_mode.Format = format;
			display_mode.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
			display_mode.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
			display_mode.Stereo = TRUE;
			return display_mode;
		}();

		return D3D12::create_swap_chain(
			*render_resources.factory,
			*render_resources.direct_command_queue,
			window.handle(),
			display_mode,
			buffer_count,
			window.fullscreen() ? 0 : 1
		);
	}

	Maia::Mythology::D3D12::Render_system create_render_system(
		Render_resources const& render_resources,
		Maia::Mythology::Win32::Window const& window,
		IDXGISwapChain3& swap_chain,
		bool const vertical_sync
	)
	{
		Eigen::Vector2i const dimensions = [&]() -> Eigen::Vector2i
		{
			Maia::Mythology::Win32::Window::Dimensions dimensions = window.dimensions();
			return { dimensions.width, dimensions.height };
		}();

		return
		{
			*render_resources.device,
			*render_resources.copy_command_queue,
			*render_resources.direct_command_queue,
			{ swap_chain, dimensions },
			3,
			vertical_sync
		};
	}

	Maia::Mythology::Application create_application(ID3D12Device& device)
	{
		auto load_scene_system = std::make_unique<Maia::Mythology::D3D12::Load_scene_system>(device);

		return Maia::Mythology::Application { std::move(load_scene_system) };
	}

	bool constexpr c_vertical_sync = false;

	struct App
	{
		Maia::Mythology::Win32::Window m_window;
		Maia::Mythology::Win32::Input_system m_input_system;

		std::unique_ptr<Render_resources> m_render_resources;
		winrt::com_ptr<IDXGISwapChain3> m_swap_chain;
		Maia::Mythology::D3D12::Render_system m_render_system;

		Maia::Mythology::Application m_application;
		
		App(HINSTANCE const instancce) :
			m_window{ instancce, main_window_process, "Mythology_win32_app", "Mythology", { 0, 0, 800, 600 } },
			m_input_system{ create_input_system(m_window.instance(), m_window.handle()) },
			m_render_resources{ std::make_unique<Render_resources>() },
			m_swap_chain{ create_swap_chain(*m_render_resources, m_window, 3, c_vertical_sync) },
			m_render_system{ create_render_system(*m_render_resources, m_window, *m_swap_chain, c_vertical_sync) },
			m_application{ create_application(*m_render_resources->device) }
		{
		}
		~App()
		{
			m_render_system.wait();
		}

		void run()
		{
			m_application.run(
				m_render_system, 
				process_all_pending_events, 
				m_input_system
			);
		}
	};
}

int WinMain(
	HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	LPSTR     lpCmdLine,
	int       nShowCmd
)
{
	std::cin.get();

	try
	{
		//GetModuleHandle(nullptr)

		App app{ hInstance };
		app.run();
	}
	catch (const std::exception& error)
	{
		std::cerr << error.what() << '\n';
		std::cin.get();
	}
	catch (...)
	{
		std::cin.get();
	}

	return 0;
}
