module;

#include <memory>

module mythology.sdl.startup_state;

import mythology.sdl.state;

namespace Mythology::SDL
{
    Startup_state::~Startup_state() noexcept
    {
    }

    std::unique_ptr<State> Startup_state::run()
    {
        // Initialize Vulkan
        // Create window
        // Load loading assets
        // Render black screen while loading assets

        // Return next state
        return {};
    }
}
