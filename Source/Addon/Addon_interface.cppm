export module mythology.addon_interface;

import maia.input;

export class Addon_interface
{
public:

    virtual ~Addon_interface() {};

    virtual void fixed_update(
        Maia::Input::Input_state const& previous_input_state,
        Maia::Input::Input_state const& current_input_state
    ) = 0;
};
