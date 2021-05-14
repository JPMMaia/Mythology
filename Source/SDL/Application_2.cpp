module;

#include <nlohmann/json.hpp>

#include <filesystem>
#include <memory>

module mythology.sdl.application_2;

import mythology.sdl.state;
import mythology.sdl.startup_state;

namespace Mythology::SDL
{
    void run(
        nlohmann::json const& pipeline_json,
        std::filesystem::path const& pipeline_json_parent_path,
        std::filesystem::path const& gltf_file_path
    ) noexcept
    {
        std::unique_ptr<State> state{std::make_unique<Startup_state>()};

        while (state != nullptr)
        {
            state = state->run();
        }
    }
}
