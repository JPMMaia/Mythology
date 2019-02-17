#ifndef MAIA_MYTHOLOGY_WIN32_WINDOW_H_INCLUDED
#define MAIA_MYTHOLOGY_WIN32_WINDOW_H_INCLUDED

#include <Windows.h>

namespace Maia::Mythology::Win32
{
	class Window
	{
	public:

		struct Dimensions
		{
			int x;
			int y;
			int width;
			int height;
		};


		Window(WNDPROC window_process, LPCSTR class_name, LPCSTR window_name);
		Window(WNDPROC window_process, LPCSTR class_name, LPCSTR window_name, Dimensions dimensions);
		~Window();

		Dimensions dimensions() const { return m_dimensions; }
		bool fullscreen() const { return m_fullscreen; }
		HWND handle() const { return m_window_handle; }

	private:

		HINSTANCE m_instance;
		LPCSTR m_class_name;
		Dimensions m_dimensions;
		HWND m_window_handle;
		bool m_fullscreen;

	};
}

#endif