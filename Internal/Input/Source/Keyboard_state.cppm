export module maia.input.keyboard_state;

import <array>;
import <cstdint>;

namespace Maia::Input
{
    export struct Keyboard_key
    {
        std::uint8_t value;
    };

    export struct Keyboard_state
    {
        bool is_down(Keyboard_key const key) const noexcept
		{
			return this->keys[key.value];
		}

		bool is_up(Keyboard_key const key) const noexcept
		{
			return !is_down(key);
		}

		void set(Keyboard_key const key, bool const isDown) noexcept
		{
			this->keys[key.value] = isDown;
		}

        // TODO use bitset
        std::array<bool, 256> keys;
    };

	export bool is_pressed(
		Keyboard_key const key,
		Keyboard_state const& previous_keyboard_state,
		Keyboard_state const& current_keyboard_state) noexcept
	{
		return current_keyboard_state.is_down(key) 
			&& previous_keyboard_state.is_up(key);
	}
	
	export bool is_released(
		Keyboard_key const key,
		Keyboard_state const& previous_keyboard_state,
		Keyboard_state const& current_keyboard_state) noexcept
	{
		return previous_keyboard_state.is_down(key) 
			&& current_keyboard_state.is_up(key);
	}
}
