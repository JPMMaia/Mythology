module;

#include <SDL2/SDL.h>
#include <vulkan/vulkan.h>

#include <memory_resource>
#include <vector>

export module mythology.sdl.vulkan;

namespace Mythology::SDL::Vulkan
{
    export std::pmr::vector<char const*> get_sdl_required_instance_extensions(
        std::pmr::polymorphic_allocator<> allocator
    );

    export struct Surface_extent
    {
        int width;
        int height;
    };

    export VkSurfaceKHR create_surface(
        SDL_Window& window,
        VkInstance const instance
    );

    export Surface_extent get_surface_extent(
        SDL_Window& window
    ) noexcept;
}
