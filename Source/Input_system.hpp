#ifndef MAIA_MYTHOLOGY_INPUTSYSTEM_H_INCLUDED
#define MAIA_MYTHOLOGY_INPUTSYSTEM_H_INCLUDED

#include <bitset>
#include <cstdint>

#include <winrt/Windows.UI.Core.h>

namespace Maia::Mythology::Input
{
	struct Keyboard_key
	{
		 winrt::Windows::System::VirtualKey value{};
	};

	struct Keyboard_state
	{
		std::bitset<256> value{};
	};

	bool is_down(Keyboard_state const& keyboard_state, Keyboard_key key)
	{
		return keyboard_state.value.test(static_cast<std::size_t>(key.value));
	}
	bool is_up(Keyboard_state const& keyboard_state, Keyboard_key key)
	{
		return !is_down(keyboard_state, key);
	}

	void set(Keyboard_state& keyboard_state, Keyboard_key key, bool value)
	{
		keyboard_state.value.set(static_cast<std::size_t>(key.value), value);
	}

	struct Input_state
	{
		Keyboard_state keyboard_previous_state{};
		Keyboard_state keyboard_current_state{};
	};

	bool is_down(Input_state const& input_state, Keyboard_key key)
	{
		return is_down(input_state.keyboard_current_state, key);
	}
	bool is_up(Input_state const& input_state, Keyboard_key key)
	{
		return is_up(input_state.keyboard_current_state, key);
	}
	bool is_pressed(Input_state const& input_state, Keyboard_key key)
	{
		return is_down(input_state.keyboard_current_state, key) && 
			is_up(input_state.keyboard_previous_state, key);
	}
	bool is_released(Input_state const& input_state, Keyboard_key key)
	{
		return is_down(input_state.keyboard_previous_state, key) &&
			is_up(input_state.keyboard_current_state, key);
	}

	void set(Input_state& input_state, Keyboard_key key, bool value)
	{
		set(input_state.keyboard_current_state, key, value);
	}

	void update(Input_state& input_state)
	{
		input_state.keyboard_previous_state = input_state.keyboard_current_state;
	}
}

#endif
