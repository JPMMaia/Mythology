export module maia.sdl.vulkan;

import <SDL2/SDL.h>;

import <vulkan/vulkan.h>;

import <memory_resource>;
import <vector>;

namespace Maia::SDL::Vulkan
{
    export std::pmr::vector<char const*> get_sdl_required_instance_extensions(
        SDL_Window& window,
        std::pmr::polymorphic_allocator<char const*> allocator = {}
    ) noexcept;

    export struct Surface_extent
    {
        int width;
        int height;
    };

    export VkSurfaceKHR create_surface(
        SDL_Window& window,
        VkInstance const instance
    ) noexcept;

    export Surface_extent get_surface_extent(
        SDL_Window& window
    ) noexcept;
}
