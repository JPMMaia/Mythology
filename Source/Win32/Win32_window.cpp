#include <stdexcept>

#include "Win32_window.hpp"

namespace Maia::Mythology::Win32
{
	namespace
	{
		HWND create_window(
			WNDPROC const window_process,
			HINSTANCE const instance,
			LPCSTR const class_name,
			LPCSTR const window_name,
			Window::Dimensions const dimensions,
			DWORD const style_flags = WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_POPUP
		)
		{
			WNDCLASSEX const window_description = [&]() -> WNDCLASSEX
			{
				WNDCLASSEX wc;
				wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
				wc.lpfnWndProc = window_process;
				wc.cbClsExtra = 0;
				wc.cbWndExtra = 0;
				wc.hInstance = instance;
				wc.hIcon = LoadIcon(nullptr, IDI_WINLOGO);
				wc.hIconSm = wc.hIcon;
				wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
				wc.hbrBackground = static_cast<HBRUSH>(GetStockObject(BLACK_BRUSH));
				wc.lpszMenuName = nullptr;
				wc.lpszClassName = class_name;
				wc.cbSize = sizeof(WNDCLASSEX);
				return wc;
			}();

			if (!RegisterClassEx(&window_description))
			{
				throw std::runtime_error("RegisterClass failed");
			}

			// Maybe apply full screen here

			HWND const window_handle =
				CreateWindowEx(
					WS_EX_APPWINDOW,
					class_name,
					window_name,
					style_flags,
					dimensions.x, dimensions.y, dimensions.width, dimensions.height,
					nullptr,
					nullptr,
					instance,
					nullptr
				);

			if (window_handle == nullptr)
			{
				throw std::runtime_error("CreateWindow failed");
			}

			ShowWindow(window_handle, SW_SHOW);
			SetForegroundWindow(window_handle);
			SetFocus(window_handle);

			ShowCursor(false);

			return window_handle;
		}

		Window::Dimensions calculate_fullscreen_dimensions()
		{
			return
			{
				0, 0,
				GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN)
			};
		}

		HWND create_fullscreen_window(
			WNDPROC const window_process,
			HINSTANCE const instance,
			LPCSTR const class_name,
			LPCSTR const window_name,
			Window::Dimensions const dimensions
		)
		{
			HWND const window_handle = create_window(
				window_process,
				instance, 
				class_name,
				window_name,
				dimensions,
				WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_POPUP | WS_CAPTION
			);

			{
				DEVMODE screen_settings = [&]() -> DEVMODE
				{
					DEVMODE screen_settings{};
					screen_settings.dmSize = sizeof(screen_settings);
					screen_settings.dmPelsWidth = static_cast<DWORD>(dimensions.width);
					screen_settings.dmPelsHeight = static_cast<DWORD>(dimensions.height);
					screen_settings.dmBitsPerPel = 32;
					screen_settings.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;
					return screen_settings;
				}();

				ChangeDisplaySettings(&screen_settings, CDS_FULLSCREEN);
			}

			return window_handle;
		}
	}

	Window::Window(WNDPROC window_process, LPCSTR class_name, LPCSTR window_name) :
		m_instance{ GetModuleHandle(nullptr) },
		m_class_name{ class_name },
		m_dimensions{ calculate_fullscreen_dimensions() },
		m_window_handle{ create_fullscreen_window(window_process, m_instance, class_name, window_name, m_dimensions) },
		m_fullscreen{ true }
	{
	}

	Window::Window(WNDPROC window_process, LPCSTR class_name, LPCSTR window_name, Dimensions dimensions) :
		m_instance{ GetModuleHandle(nullptr) },
		m_class_name{ class_name },
		m_dimensions{ dimensions },
		m_window_handle{ create_window(window_process, m_instance, class_name, window_name, dimensions) },
		m_fullscreen{ false }
	{
	}

	Window::~Window()
	{
		ShowCursor(true);

		if (m_fullscreen)
		{
			ChangeDisplaySettings(nullptr, 0);
		}

		DestroyWindow(m_window_handle);
		UnregisterClass(m_class_name, m_instance);
	}
}
