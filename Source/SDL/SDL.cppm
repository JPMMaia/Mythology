module;

#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>

#include <memory_resource>
#include <span>
#include <vector>

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

    export std::pmr::vector<SDL_Window*> get_window_raw_pointers(
        std::span<SDL_window const> windows,
        std::pmr::polymorphic_allocator<> const& allocator
    );
}
