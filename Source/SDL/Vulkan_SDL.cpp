module;

#include <SDL2/SDL.h>
#include <SDL2/SDL_vulkan.h>
#include <vulkan/vulkan.h>

#include <cassert>
#include <iostream>
#include <memory_resource>
#include <span>
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

    namespace
    {
        std::pmr::vector<VkSurfaceKHR> create_surfaces(
            VkInstance const instance,
            std::span<SDL_Window* const> const windows,
            std::pmr::polymorphic_allocator<> const& allocator
        )
        {
            std::pmr::vector<VkSurfaceKHR> surfaces{allocator};
            surfaces.reserve(windows.size());

            for (SDL_Window* const window : windows)
            {
                VkSurfaceKHR const surface =
                    window != nullptr ?
                    create_surface(*window, instance) :
                    VkSurfaceKHR{VK_NULL_HANDLE};

                surfaces.push_back(surface);
            }

            return surfaces;
        }
    }

    Surface_resources::Surface_resources(
        VkInstance const instance,
        std::span<SDL_Window* const> windows,
        std::pmr::polymorphic_allocator<> const& allocator
    ) :
        instance{instance},
        surfaces{create_surfaces(instance, windows, allocator)}
    {
    }

    Surface_resources::Surface_resources(Surface_resources&& other) noexcept :
        instance{std::exchange(other.instance, VkInstance{VK_NULL_HANDLE})},
        surfaces{std::move(other.surfaces)}
    {
    }

    Surface_resources::~Surface_resources() noexcept
    {
        for (VkSurfaceKHR const surface : this->surfaces)
        {
            vkDestroySurfaceKHR(this->instance, surface, nullptr);
        }
    }

    Surface_resources& Surface_resources::operator=(Surface_resources&& other) noexcept
    {
        std::swap(this->instance, other.instance);
        std::swap(this->surfaces, other.surfaces);

        return *this;
    }
}
