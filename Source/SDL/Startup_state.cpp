module;

#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>

#include <array>
#include <cassert>
#include <memory>
#include <memory_resource>
#include <span>
#include <string>
#include <variant>
#include <vector>

module mythology.sdl.startup_state;

import maia.renderer.vulkan;

import mythology.sdl.render_resources;
import mythology.sdl.sdl;
import mythology.sdl.state;
import mythology.sdl.vulkan;
namespace Mythology::SDL
{
    Startup_state::~Startup_state() noexcept
    {
    }

    struct Window_offset
    {
        int x = 0;
        int y = 0;
    };

    struct Window_extent
    {
        int width = 0;
        int height = 0;
    };

    struct Fullscreen_mode
    {
        unsigned int display_index;
    };

    struct Windowed_mode
    {
        Window_offset offset;
        Window_extent extent;
    };

    struct Window_configuration
    {
        std::pmr::string title;
        std::variant<Fullscreen_mode, Windowed_mode> mode;
    };

    namespace
    {
        std::pmr::vector<SDL_window> create_windows(
            SDL_instance const& sdl,
            std::span<Window_configuration const> const window_configurations
        )
        {
            std::pmr::vector<SDL_window> windows;
            windows.reserve(window_configurations.size());

            constexpr Uint32 common_flags = SDL_WINDOW_VULKAN;

            int const number_of_displays = SDL_GetNumVideoDisplays();

            for (Window_configuration const& configuration : window_configurations)
            {
                if (configuration.mode.index() == 0)
                {
                    Fullscreen_mode const& fullscreen_mode = std::get<Fullscreen_mode>(configuration.mode);

                    if (fullscreen_mode.display_index < number_of_displays)
                    {
                        SDL_Rect bounds = {};
                        SDL_GetDisplayBounds(fullscreen_mode.display_index, &bounds);

                        windows.push_back(
                            SDL_window
                            {
                                sdl,
                                configuration.title.c_str(),
                                bounds.x,
                                bounds.y,
                                bounds.w,
                                bounds.h,
                                SDL_WINDOW_FULLSCREEN_DESKTOP | common_flags
                            }
                        );
                    }
                }
                else
                {
                    assert(configuration.mode.index() == 1);

                    Windowed_mode const& windowed_mode = std::get<Windowed_mode>(configuration.mode);

                    windows.push_back(
                        SDL_window
                        {
                            sdl,
                            configuration.title.c_str(),
                            windowed_mode.offset.x,
                            windowed_mode.offset.y,
                            windowed_mode.extent.width,
                            windowed_mode.extent.height,
                            common_flags
                        }
                    );
                }
            }

            return windows;
        }
    }

    std::unique_ptr<State> Startup_state::run()
    {
        std::array<Window_configuration, 1> const window_configurations
        {
            Window_configuration
            {
                .title = "Main window",
                //.mode = Fullscreen_mode{.display_index = 0},
                .mode = Windowed_mode
                {
                    .offset = {0, 0},
                    .extent = {800, 600},
                },
            }
        };

        SDL_instance sdl{SDL_INIT_VIDEO};

        std::pmr::vector<SDL_window> const windows = create_windows(sdl, window_configurations);

        std::pmr::vector<char const*> const required_instance_extensions =
            Mythology::SDL::Vulkan::get_sdl_required_instance_extensions({});

        Mythology::Render::Instance_resources instance_resources
        {
            Maia::Renderer::Vulkan::make_api_version(1, 2, 0),
            required_instance_extensions
        };

        {
            using namespace std::chrono_literals;
            std::this_thread::sleep_for(5s);
        }

        // Create surfaces
        // Choose which physical devices and queue family indices to use
        // Create devices
        // Get queues
        // Create swapchains
        // Create swapchains image views
        // Create semaphores
        // Create fences

        // Load loading assets
        // Render black screen while loading assets

        

        // Return next state
        return {};
    }
}
