export module mythology.windowless;

import <filesystem>;

namespace Mythology::Windowless
{
    export void render_frame(
        std::filesystem::path const& output_filename
    ) noexcept;
}