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

        Example_addon_interface() { std::cout << "hello from dll!" << std::endl; }
        ~Example_addon_interface() {}

        void fixed_update(
            Maia::Input::Input_state const& previous_input_state,
            Maia::Input::Input_state const& current_input_state
        ) final {}
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
