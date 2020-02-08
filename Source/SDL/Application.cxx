export module mythology.sdl.application;

import <filesystem>;

namespace Mythology::SDL
{
    export void run_with_window() noexcept;

    export void render_frame(
        std::filesystem::path const& output_filename
    ) noexcept;
}