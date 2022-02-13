#define MYTHOLOGY_ADDON_IMPLEMENTATION
#include <Addon_interface.hpp>

#include <iostream>

import maia.input;
import mythology.addon_interface;

namespace Mythology::Addons
{
    class Example_addon_interface final : public Addon_interface
    {
    public:

        Example_addon_interface() {}
        ~Example_addon_interface() {}

        void process_input(
            std::optional<Maia::Input::Keyboard_state> const current_keyboard_state,
            std::optional<Maia::Input::Mouse_state> const current_mouse_state,
            std::span<Maia::Input::Game_controller_state const> const current_game_controllers_state
        ) final
        {

        }

        void fixed_update() final
        {

        }
    };
}

MYTHOLOGY_ADDON_API Addon_interface* create_addon_interface()
{
    return new Mythology::Addons::Example_addon_interface{};
}

MYTHOLOGY_ADDON_API void destroy_addon_interface(Addon_interface* const addon_interface)
{
    delete addon_interface;
}
