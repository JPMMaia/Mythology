module;

#include <nlohmann/json.hpp>

#include <filesystem>
#include <memory>
#include <memory_resource>
#include <string>
#include <unordered_map>

module mythology.sdl.application_2;

import mythology.sdl.state;
import mythology.sdl.startup_state;

namespace Mythology::SDL
{
    void run(
        std::pmr::unordered_map<std::pmr::string, std::filesystem::path> const& render_pipelines_configurations,
        std::filesystem::path const& gltf_file_path
    )
    {
        std::unique_ptr<State> state{std::make_unique<Startup_state>(render_pipelines_configurations)};

        while (state != nullptr)
        {
            state = state->run();
        }
    }
}
