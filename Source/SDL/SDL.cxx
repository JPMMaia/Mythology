module;

#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>

export module mythology.sdl.sdl;

namespace Mythology::SDL
{
    export class SDL_instance
    {
    public:

        SDL_instance(Uint32 flags);
        SDL_instance(SDL_instance const&) noexcept = delete;
        SDL_instance(SDL_instance&& other) noexcept;
        ~SDL_instance() noexcept;

        SDL_instance& operator=(SDL_instance const&) noexcept = delete;
        SDL_instance& operator=(SDL_instance&&) noexcept = delete;

    private:

        bool m_was_moved = false;

    };

    export class SDL_window
    {
    public:

        SDL_window(
            SDL_instance const& sdl,
            char const* const title,
            int const x,
            int const y,
            int const w,
            int const h,
            Uint32 const flags
        );
        SDL_window(SDL_window const&) noexcept = delete;
        SDL_window(SDL_window&& other) noexcept;
        ~SDL_window() noexcept;

        SDL_window& operator=(SDL_window const&) noexcept = delete;
        SDL_window& operator=(SDL_window&& other) noexcept;

        SDL_Window* get() const noexcept;

    private:

        SDL_Window* m_window = nullptr;

    };
}
