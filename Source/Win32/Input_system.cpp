#include <cstdint>
#include <iostream>
#include <optional>

#include "Input_system.hpp"

namespace Maia::Mythology::Win32
{
	namespace
	{
		void check_hresult(HRESULT result)
		{
			if (FAILED(result))
			{
				{
					winrt::hresult_error error{ result };
					std::cerr << winrt::to_string(error.message()) << '\n';
				}

				check_hresult(result);
			}
		}

		struct Keyboard_state
		{
			std::array<std::uint8_t, 256> value;
		};

		struct Mouse_state
		{
			DIMOUSESTATE2 value;
		};

		winrt::com_ptr<IDirectInput8> create_direct_input(HINSTANCE instance)
		{
			winrt::com_ptr<IDirectInput8> direct_input;

			check_hresult(
				DirectInput8Create(
					instance,
					DIRECTINPUT_VERSION,
					IID_IDirectInput8,
					direct_input.put_void(),
					nullptr
				)
			);

			return direct_input;
		}

		winrt::com_ptr<IDirectInputDevice8> create_keyboard(IDirectInput8& direct_input, HWND const window_handle)
		{
			winrt::com_ptr<IDirectInputDevice8> keyboard;

			check_hresult(
				direct_input.CreateDevice(GUID_SysKeyboard, keyboard.put(), nullptr));

			check_hresult(
				keyboard->SetDataFormat(&c_dfDIKeyboard));

			check_hresult(
				keyboard->SetCooperativeLevel(window_handle, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE));

			return keyboard;
		}

		winrt::com_ptr<IDirectInputDevice8> create_mouse(IDirectInput8& direct_input, HWND const window_handle)
		{
			winrt::com_ptr<IDirectInputDevice8> mouse;

			check_hresult(
				direct_input.CreateDevice(GUID_SysMouse, mouse.put(), nullptr));

			check_hresult(
				mouse->SetDataFormat(&c_dfDIMouse2));

			check_hresult(
				mouse->SetCooperativeLevel(window_handle, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE));

			return mouse;
		}
	}

	Input_system::Input_system(HINSTANCE const instance, HWND const window_handle, const std::array<std::uint8_t, 256>& keys_map) :
		m_direct_input{ create_direct_input(instance) },
		m_keyboard{ create_keyboard(*m_direct_input, window_handle) },
		m_mouse{ create_mouse(*m_direct_input, window_handle) },
		m_keys_map{ keys_map },
		m_input_state{}
	{
		m_keyboard->Acquire();
		m_mouse->Acquire();
	}
	Input_system::~Input_system()
	{
		if (m_mouse)
		{
			m_mouse->Unacquire();
		}

		if (m_keyboard)
		{
			m_keyboard->Unacquire();
		}
	}

	namespace
	{
		Keyboard_state read_keyboard(IDirectInputDevice8& keyboard)
		{
			Keyboard_state keyboard_state;

			HRESULT const result =
				keyboard.GetDeviceState(
					static_cast<DWORD>(keyboard_state.value.size() * sizeof(decltype(Keyboard_state::value)::value_type)),
					keyboard_state.value.data()
				);

			if (FAILED(result))
			{
				if ((result == DIERR_INPUTLOST) || (result == DIERR_NOTACQUIRED))
				{
					keyboard.Acquire();
				}
				else
				{
					check_hresult(result);
				}

				return {};
			}

			return keyboard_state;
		}

		Mouse_state read_mouse(IDirectInputDevice8& mouse)
		{
			Mouse_state mouse_state;

			HRESULT const result =
				mouse.GetDeviceState(sizeof(mouse_state.value), &mouse_state.value);

			if (FAILED(result))
			{
				if ((result == DIERR_INPUTLOST) || (result == DIERR_NOTACQUIRED))
				{
					mouse.Acquire();
				}
				else
				{
					check_hresult(result);
				}

				return {};
			}

			return mouse_state;
		}
	}

	Maia::Mythology::Input::Input_state const& Input_system::execute()
	{
		/*m_input_state.overwrite_previous_with_current_state();

		{
			Keyboard_state const keyboard_state = read_keyboard(*m_keyboard);

			std::bitset<256>& keyboard_bitset_state = m_input_state.keys_current_state.value;

			for (std::size_t index = 0; index < keyboard_state.value.size(); ++index)
			{
				std::uint8_t const mapped_key = m_keys_map[index];
				bool const key_state = (keyboard_state.value[index] & 0x80) != 0;

				keyboard_bitset_state.set(mapped_key, key_state);
			}
		}

		{
			Mouse_state const mouse_state = read_mouse(*m_mouse);

			{
				Eigen::Vector2i& mouse_position = m_input_state.mouse_current_state.position;
				mouse_position(0) += mouse_state.value.lX;
				mouse_position(1) += mouse_state.value.lY;
			}

			{
				Eigen::Vector2i& mouse_delta = m_input_state.mouse_current_state.delta;
				mouse_delta = { mouse_state.value.lX, mouse_state.value.lY };
			}
		}

		return m_input_state;*/
		return {};
	}
}
