module;

#include <SDL2/SDL.h>
#include <SDL2/SDL_vulkan.h>
#include <vulkan/vulkan.hpp>

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

    PFN_vkGetInstanceProcAddr get_instance_process_address() noexcept
    {
        return static_cast<PFN_vkGetInstanceProcAddr>(SDL_Vulkan_GetVkGetInstanceProcAddr());
    }

    vk::SurfaceKHR create_surface(
        SDL_Window& window,
        vk::Instance const instance
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
        std::pmr::vector<vk::SurfaceKHR> create_surfaces(
            vk::Instance const instance,
            std::span<SDL_Window* const> const windows,
            std::pmr::polymorphic_allocator<> const& allocator
        )
        {
            std::pmr::vector<vk::SurfaceKHR> surfaces{allocator};
            surfaces.reserve(windows.size());

            for (SDL_Window* const window : windows)
            {
                vk::SurfaceKHR const surface =
                    window != nullptr ?
                    create_surface(*window, instance) :
                    vk::SurfaceKHR{};

                surfaces.push_back(surface);
            }

            return surfaces;
        }
    }

    Surface_resources::Surface_resources(
        vk::Instance const instance,
        std::span<SDL_Window* const> windows,
        std::pmr::polymorphic_allocator<> const& allocator
    ) :
        instance{instance},
        surfaces{create_surfaces(instance, windows, allocator)}
    {
    }

    Surface_resources::Surface_resources(Surface_resources&& other) noexcept :
        instance{std::exchange(other.instance, vk::Instance{})},
        surfaces{std::move(other.surfaces)}
    {
    }

    Surface_resources::~Surface_resources() noexcept
    {
        for (vk::SurfaceKHR const surface : this->surfaces)
        {
            this->instance.destroy(surface, nullptr);
        }
    }

    Surface_resources& Surface_resources::operator=(Surface_resources&& other) noexcept
    {
        std::swap(this->instance, other.instance);
        std::swap(this->surfaces, other.surfaces);

        return *this;
    }
}
