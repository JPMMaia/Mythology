export module maia.input.game_controller_state;

import <array>;
import <cstdint>;

namespace Maia::Input
{
    export struct Game_controller_button
    {
        std::uint8_t value;
    };

    export struct Game_controller_axis
    {
        std::int16_t x;
        std::int16_t y;
    };

    export struct Game_controller_trigger
    {
        std::int16_t value;
    };

    export struct Game_controller_state
    {
        bool is_down(Game_controller_button const button) const noexcept
		{
			return this->buttons[button.value];
		}

		bool is_up(Game_controller_button const button) const noexcept
		{
			return !is_down(button);
		}

        Game_controller_axis left_axis;
        Game_controller_axis right_axis;
        Game_controller_trigger left_trigger;
        Game_controller_trigger right_trigger;
        std::array<bool, 15> buttons;
    };

    export bool is_pressed(
		Game_controller_button const button,
		Game_controller_state const& previous_game_controller_state,
		Game_controller_state const& current_game_controller_state) noexcept
	{
		return current_game_controller_state.is_down(button) 
			&& previous_game_controller_state.is_up(button);
	}
	
	export bool is_released(
		Game_controller_button const button,
		Game_controller_state const& previous_game_controller_state,
		Game_controller_state const& current_game_controller_state) noexcept
	{
		return previous_game_controller_state.is_down(button) 
			&& current_game_controller_state.is_up(button);
	}

    export struct Game_controller_axis_delta
	{
		int x;
		int y;
	};

	export Game_controller_axis_delta game_controller_axis_delta(
        Game_controller_axis const previous_game_controller_axis,
        Game_controller_axis const current_game_controller_axis) noexcept
	{
		return
        {
            current_game_controller_axis.x - previous_game_controller_axis.x,
            current_game_controller_axis.y - previous_game_controller_axis.y
        };
	}

    export struct Game_controller_trigger_delta
	{
		int x;
		int y;
	};

	export Game_controller_trigger_delta game_controller_axis_delta(
        Game_controller_trigger const previous_game_controller_trigger,
        Game_controller_trigger const current_game_controller_trigger) noexcept
	{
		return {current_game_controller_trigger.value - previous_game_controller_trigger.value};
	}
}
