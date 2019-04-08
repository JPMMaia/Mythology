#ifndef MAIA_MYTHOLOGY_WIN32_INPUTSYSTEM_H_INCLUDED
#define MAIA_MYTHOLOGY_WIN32_INPUTSYSTEM_H_INCLUDED

#define DIRECTINPUT_VERSION 0x0800

#pragma comment(lib, "dinput8.lib")
#pragma comment(lib, "dxguid.lib")

#include <dinput.h>

#include <winrt/base.h>

#include <IInput_system.hpp>

#include <Input_state.hpp>

namespace Maia::Mythology::Win32
{
	class Input_system final : public Maia::Mythology::Input::IInput_system
	{
	public:

		Input_system(HINSTANCE const instance, HWND const window_handle, const std::array<std::uint8_t, 256>& keys_map);
		~Input_system();


		Maia::Mythology::Input::Input_state const& execute() final;


	private:

		winrt::com_ptr<IDirectInput8> m_direct_input;
		winrt::com_ptr<IDirectInputDevice8> m_keyboard;
		winrt::com_ptr<IDirectInputDevice8> m_mouse;

		std::array<std::uint8_t, 256> m_keys_map;

		Maia::Mythology::Input::Input_state m_input_state{};

	};
}

#endif