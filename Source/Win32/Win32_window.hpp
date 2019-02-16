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

	private:

		HINSTANCE m_instance;
		LPCSTR m_class_name;
		HWND m_window_handle;
		bool m_fullscreen;

	};
}