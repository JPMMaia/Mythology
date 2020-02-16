module maia.sdl.vulkan;

import <SDL2/SDL.h>;
import <SDL2/SDL_vulkan.h>;

import <vulkan/vulkan.h>;

import <cassert>;
import <iostream>;
import <memory_resource>;
import <vector>;

namespace Maia::SDL::Vulkan
{
    namespace
    {
        void check(SDL_bool const status) noexcept
        {
            if (status != SDL_TRUE)
            {
                std::cerr << SDL_GetError() << '\n';
            }

            assert(status == SDL_TRUE);
        }
    }

    std::pmr::vector<char const*> get_sdl_required_instance_extensions(
        SDL_Window& window,
        std::pmr::polymorphic_allocator<char const*> allocator
    ) noexcept
    {
        unsigned int count = 0;
        check(
            SDL_Vulkan_GetInstanceExtensions(&window, &count, nullptr));

        std::pmr::vector<char const*> instance_extensions{allocator};
        instance_extensions.resize(count);

        check(
            SDL_Vulkan_GetInstanceExtensions(&window, &count, instance_extensions.data()));

        return instance_extensions;
    }

    VkSurfaceKHR create_surface(
        SDL_Window& window,
        VkInstance const instance
    ) noexcept
    {
        VkSurfaceKHR surface = {};
        check(
            SDL_Vulkan_CreateSurface(&window, instance, &surface));

        return surface;
    }

    Surface_extent get_surface_extent(
        SDL_Window& window
    ) noexcept
    {        
        int width = 0;
        int height = 0;
        SDL_Vulkan_GetDrawableSize(&window, &width, &height);

        return {width, height};
    }
}
