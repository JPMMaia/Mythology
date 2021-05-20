module;

#include <SDL2/SDL.h>
#include <SDL2/SDL_vulkan.h>
#include <vulkan/vulkan.h>

#include <cassert>
#include <iostream>
#include <memory_resource>
#include <stdexcept>
#include <vector>

module mythology.sdl.vulkan;

namespace Mythology::SDL::Vulkan
{
    namespace
    {
        void check(SDL_bool const status)
        {
            if (status != SDL_TRUE)
            {
                std::cerr << SDL_GetError() << '\n';

                throw std::runtime_error{SDL_GetError()};
            }
        }
    }

    std::pmr::vector<char const*> get_sdl_required_instance_extensions(
        std::pmr::polymorphic_allocator<> allocator
    )
    {
        unsigned int count = 0;
        check(
            SDL_Vulkan_GetInstanceExtensions(nullptr, &count, nullptr));

        std::pmr::vector<char const*> instance_extensions{allocator};
        instance_extensions.resize(count);

        check(
            SDL_Vulkan_GetInstanceExtensions(nullptr, &count, instance_extensions.data()));

        return instance_extensions;
    }

    VkSurfaceKHR create_surface(
        SDL_Window& window,
        VkInstance const instance
    )
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
