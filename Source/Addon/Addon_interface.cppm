module;

#include <optional>
#include <span>

export module mythology.addon_interface;

import maia.input;

export class Addon_interface
{
public:

    virtual ~Addon_interface() {};

    virtual void process_input(
        std::optional<Maia::Input::Keyboard_state> const current_keyboard_state,
        std::optional<Maia::Input::Mouse_state> const current_mouse_state,
        std::span<Maia::Input::Game_controller_state const> const current_game_controllers_state
    ) = 0;

    virtual void fixed_update() = 0;
};
