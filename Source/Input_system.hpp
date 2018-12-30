#ifndef MAIA_MYTHOLOGY_INPUTSYSTEM_H_INCLUDED
#define MAIA_MYTHOLOGY_INPUTSYSTEM_H_INCLUDED

#include <bitset>
#include <cstdint>

#include <winrt/Windows.UI.Core.h>

namespace Maia::Mythology::Input
{
	struct Key
	{
		 winrt::Windows::System::VirtualKey value{};
	};

	struct Keys_state
	{
		std::bitset<256> value{};
	};

	bool is_down(Keys_state const& keys_state, Key key)
	{
		return keys_state.value.test(static_cast<std::size_t>(key.value));
	}
	bool is_up(Keys_state const& keys_state, Key key)
	{
		return !is_down(keys_state, key);
	}

	void set(Keys_state& keys_state, Key key, bool value)
	{
		keys_state.value.set(static_cast<std::size_t>(key.value), value);
	}


	struct Mouse_state
	{
		Eigen::Vector2f position{ 0.0f, 0.0f };
	};
	

	struct Input_state
	{
		Keys_state keys_previous_state{};
		Keys_state keys_current_state{};
		Mouse_state mouse_previous_state{};
		Mouse_state mouse_current_state{};
	};

	bool is_down(Input_state const& input_state, Key key)
	{
		return is_down(input_state.keys_current_state, key);
	}
	bool is_up(Input_state const& input_state, Key key)
	{
		return is_up(input_state.keys_current_state, key);
	}
	bool is_pressed(Input_state const& input_state, Key key)
	{
		return is_down(input_state.keys_current_state, key) && 
			is_up(input_state.keys_previous_state, key);
	}
	bool is_released(Input_state const& input_state, Key key)
	{
		return is_down(input_state.keys_previous_state, key) &&
			is_up(input_state.keys_current_state, key);
	}

	Eigen::Vector2f delta_mouse_position(Input_state const& input_state)
	{
		return input_state.mouse_current_state.position - input_state.mouse_previous_state.position;
	}

	void set(Input_state& input_state, Key key, bool value)
	{
		set(input_state.keys_current_state, key, value);
	}
	void set(Input_state& input_state, Eigen::Vector2f mouse_position)
	{
		input_state.mouse_current_state.position = mouse_position;
	}

	void update(Input_state& input_state)
	{
		input_state.keys_previous_state = input_state.keys_current_state;
	}
}

#endif
