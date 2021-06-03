module;

#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>

#include <algorithm>
#include <iostream>
#include <memory_resource>
#include <span>
#include <stdexcept>
#include <utility>
#include <vector>

module mythology.sdl.sdl;

namespace Mythology::SDL
{
    SDL_instance::SDL_instance(Uint32 const flags) :
        m_was_moved{false}
    {
        SDL_SetMainReady();

        if (SDL_Init(flags) != 0)
        {
            std::cerr << SDL_GetError() << std::endl;
            throw std::runtime_error{SDL_GetError()};
        }
    }

    SDL_instance::SDL_instance(SDL_instance&& other) noexcept :
        m_was_moved{false}
    {
        other.m_was_moved = true;
    }
    
    SDL_instance::~SDL_instance() noexcept
    {
        if (!m_was_moved)
        {
            SDL_Quit();
        }
    }

    SDL_window::SDL_window(
        SDL_instance const& sdl,
        char const* const title,
        int const x,
        int const y,
        int const w,
        int const h,
        Uint32 const flags
    ) :
        m_window{SDL_CreateWindow(title, x, y, w, h, flags)}
    {
        if (m_window == nullptr)
        {
            std::cerr << SDL_GetError() << std::endl;
            throw std::runtime_error{SDL_GetError()};
        }
    }

    SDL_window::SDL_window(SDL_window&& other) noexcept :
        m_window{std::exchange(other.m_window, nullptr)}
    {
    }

    SDL_window::~SDL_window() noexcept
    {
        if (m_window != nullptr)
        {
            SDL_DestroyWindow(m_window);
        }
    }

    SDL_window& SDL_window::operator=(SDL_window&& other) noexcept
    {
        std::swap(m_window, other.m_window);
        
        return *this;
    }

    SDL_Window* SDL_window::get() const noexcept
    {
        return m_window;
    }

    std::pmr::vector<SDL_Window*> get_window_raw_pointers(
        std::span<SDL_window const> const windows,
        std::pmr::polymorphic_allocator<> const& allocator
    )
    {
        std::pmr::vector<SDL_Window*> window_raw_pointers{allocator};
        window_raw_pointers.resize(windows.size(), nullptr);

        auto const to_raw_pointer = [] (SDL_window const& window) -> SDL_Window*
        {
            return window.get();
        };

        std::transform(windows.begin(), windows.end(), window_raw_pointers.begin(), to_raw_pointer);

        return window_raw_pointers;
    }
}
