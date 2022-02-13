module;

#include <SDL2/SDL.h>

#include <memory_resource>
#include <span>
#include <vector>

export module mythology.sdl.input;

import maia.input;

namespace Mythology::SDL
{
    export Maia::Input::Keyboard_state get_keyboard_state() noexcept;

    export Maia::Input::Mouse_state get_mouse_state() noexcept;

    export struct Game_controller
    {
        Game_controller(int const joystick_index) noexcept;
        Game_controller(Game_controller const&) noexcept = delete;
        Game_controller(Game_controller&& other) noexcept;
        ~Game_controller() noexcept;

        Game_controller& operator=(Game_controller const&) noexcept = delete;
        Game_controller& operator=(Game_controller&& other) noexcept;

        SDL_GameController* value;
    };

    export Maia::Input::Game_controller_state get_game_controller_state(SDL_GameController& game_controller) noexcept;

    export std::pmr::vector<Maia::Input::Game_controller_state> get_game_controllers_state(
        std::span<Game_controller const> const game_controllers,
        std::pmr::polymorphic_allocator<> const& output_allocator
    );

    export void add_game_controller(
        Sint32 const added_instance_id,
        std::pmr::vector<Game_controller>& game_controllers
    );

    export void remove_game_controller(
        Sint32 const removed_instance_id,
        std::pmr::vector<Game_controller>& game_controllers
    );
}