export module maia.input.mouse_state;

import <array>;
import <cstdint>;

namespace Maia::Input
{
    export struct Mouse_key
    {
        std::uint8_t value;
    };

    export struct Mouse_position
    {
        int x;
        int y;
    };

    export struct Mouse_state
    {
        bool is_down(Mouse_key const key) const noexcept
		{
			return this->keys[key.value];
		}

		bool is_up(Mouse_key const key) const noexcept
		{
			return !is_down(key);
		}

		void set(Mouse_key const key, bool const isDown) noexcept
		{
			this->keys[key.value] = isDown;
		}

        Mouse_position position;
        std::array<bool, 8> keys;
    };

    export bool is_pressed(
		Mouse_key const key,
		Mouse_state const& previous_mouse_state,
		Mouse_state const& current_mouse_state) noexcept
	{
		return current_mouse_state.is_down(key) 
			&& previous_mouse_state.is_up(key);
	}
	
	export bool is_released(
		Mouse_key const key,
		Mouse_state const& previous_mouse_state,
		Mouse_state const& current_mouse_state) noexcept
	{
		return previous_mouse_state.is_down(key) 
			&& current_mouse_state.is_up(key);
	}

    export struct Mouse_delta_position
	{
		int x;
		int y;
	};

	export Mouse_delta_position mouse_delta_position(
        Mouse_position const previous_mouse_position,
        Mouse_position const current_mouse_position) noexcept
	{
		return
        {
            current_mouse_position.x - previous_mouse_position.x,
            current_mouse_position.y - previous_mouse_position.y
        };
	}
}
