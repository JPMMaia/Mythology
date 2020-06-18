export module mythology.sdl.application;

import <nlohmann/json.hpp>;

namespace Mythology::SDL
{
    export void run(nlohmann::json const& pipeline_json) noexcept;
}
