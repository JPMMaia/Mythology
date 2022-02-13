module;

#include <SDL2/SDL.h>

#include <memory_resource>
#include <vector>

module mythology.sdl.input;

import maia.input;

namespace Mythology::SDL
{
    Maia::Input::Keyboard_state get_keyboard_state() noexcept
    {
        Uint8 const* const sdl_keyboard_state = SDL_GetKeyboardState(nullptr);

        Maia::Input::Keyboard_state maia_keyboard_state;
        std::transform(sdl_keyboard_state, sdl_keyboard_state + 256, maia_keyboard_state.keys.begin(),
            [](Uint8 const state) -> bool { return state == 1; });

        return maia_keyboard_state;
    }

    Maia::Input::Mouse_state get_mouse_state() noexcept
    {
        int x = 0, y = 0;
        Uint32 const state = SDL_GetMouseState(&x, &y);

        return Maia::Input::Mouse_state
        {
            .position = {x, y},
            .keys = {
                (state & SDL_BUTTON(SDL_BUTTON_LEFT)) != 0 ? true : false,
                (state & SDL_BUTTON(SDL_BUTTON_MIDDLE)) != 0 ? true : false,
                (state & SDL_BUTTON(SDL_BUTTON_RIGHT)) != 0 ? true : false,
                (state & SDL_BUTTON(SDL_BUTTON_X1)) != 0 ? true : false,
                (state & SDL_BUTTON(SDL_BUTTON_X2)) != 0 ? true : false,
                false,
                false,
                false
            }
        };
    }

    Game_controller::Game_controller(int const joystick_index) noexcept :
        value{ SDL_GameControllerOpen(joystick_index) }
    {
    }

    Game_controller::Game_controller(Game_controller&& other) noexcept :
        value{ std::exchange(other.value, nullptr) }
    {
    }

    Game_controller::~Game_controller() noexcept
    {
        if (this->value != nullptr)
        {
            SDL_GameControllerClose(this->value);
        }
    }

    Game_controller& Game_controller::operator = (Game_controller&& other) noexcept
    {
        this->value = std::exchange(other.value, nullptr);

        return *this;
    }

    Maia::Input::Game_controller_state get_game_controller_state(SDL_GameController& game_controller) noexcept
    {
        return {
            .left_axis = {
                .x = SDL_GameControllerGetAxis(&game_controller, SDL_CONTROLLER_AXIS_LEFTX),
                .y = SDL_GameControllerGetAxis(&game_controller, SDL_CONTROLLER_AXIS_LEFTY)
            },
            .right_axis = {
                .x = SDL_GameControllerGetAxis(&game_controller, SDL_CONTROLLER_AXIS_RIGHTX),
                .y = SDL_GameControllerGetAxis(&game_controller, SDL_CONTROLLER_AXIS_RIGHTY)
            },
            .left_trigger = {SDL_GameControllerGetAxis(&game_controller, SDL_CONTROLLER_AXIS_TRIGGERLEFT)},
            .right_trigger = {SDL_GameControllerGetAxis(&game_controller, SDL_CONTROLLER_AXIS_TRIGGERRIGHT)},
            .buttons = {
                SDL_GameControllerGetButton(&game_controller, SDL_CONTROLLER_BUTTON_A) == 1,
                SDL_GameControllerGetButton(&game_controller, SDL_CONTROLLER_BUTTON_B) == 1,
                SDL_GameControllerGetButton(&game_controller, SDL_CONTROLLER_BUTTON_X) == 1,
                SDL_GameControllerGetButton(&game_controller, SDL_CONTROLLER_BUTTON_Y) == 1,
                SDL_GameControllerGetButton(&game_controller, SDL_CONTROLLER_BUTTON_BACK) == 1,
                SDL_GameControllerGetButton(&game_controller, SDL_CONTROLLER_BUTTON_GUIDE) == 1,
                SDL_GameControllerGetButton(&game_controller, SDL_CONTROLLER_BUTTON_START) == 1,
                SDL_GameControllerGetButton(&game_controller, SDL_CONTROLLER_BUTTON_LEFTSTICK) == 1,
                SDL_GameControllerGetButton(&game_controller, SDL_CONTROLLER_BUTTON_RIGHTSTICK) == 1,
                SDL_GameControllerGetButton(&game_controller, SDL_CONTROLLER_BUTTON_LEFTSHOULDER) == 1,
                SDL_GameControllerGetButton(&game_controller, SDL_CONTROLLER_BUTTON_RIGHTSHOULDER) == 1,
                SDL_GameControllerGetButton(&game_controller, SDL_CONTROLLER_BUTTON_DPAD_UP) == 1,
                SDL_GameControllerGetButton(&game_controller, SDL_CONTROLLER_BUTTON_DPAD_DOWN) == 1,
                SDL_GameControllerGetButton(&game_controller, SDL_CONTROLLER_BUTTON_DPAD_LEFT) == 1,
                SDL_GameControllerGetButton(&game_controller, SDL_CONTROLLER_BUTTON_DPAD_RIGHT) == 1
            }
        };
    }

    std::pmr::vector<Maia::Input::Game_controller_state> get_game_controllers_state(
        std::span<Game_controller const> const game_controllers,
        std::pmr::polymorphic_allocator<> const& output_allocator
    )
    {
        std::pmr::vector<Maia::Input::Game_controller_state> game_controllers_state{ output_allocator };
        game_controllers_state.resize(game_controllers.size());

        for (std::size_t game_controller_index = 0; game_controller_index < game_controllers.size(); ++game_controller_index)
        {
            game_controllers_state[game_controller_index] = get_game_controller_state(*game_controllers[game_controller_index].value);
        }

        return game_controllers_state;
    }

    void add_game_controller(
        Sint32 const added_instance_id,
        std::pmr::vector<Game_controller>& game_controllers
    )
    {
        game_controllers.emplace_back(added_instance_id);
    }

    void remove_game_controller(
        Sint32 const removed_instance_id,
        std::pmr::vector<Game_controller>& game_controllers
    )
    {
        auto const is_game_controller_with_id = [removed_instance_id](Game_controller const& game_controller) -> bool
        {
            SDL_Joystick* const joystick = SDL_GameControllerGetJoystick(game_controller.value);
            SDL_JoystickID const instance_id = SDL_JoystickInstanceID(joystick);

            return instance_id == removed_instance_id;
        };

        auto const game_controller = std::find_if(game_controllers.begin(), game_controllers.end(), is_game_controller_with_id);
        if (game_controller != game_controllers.end())
        {
            game_controllers.erase(game_controller);
        }
    }
}