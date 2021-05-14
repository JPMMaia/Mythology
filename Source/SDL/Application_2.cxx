module;

#include <nlohmann/json.hpp>

#include <filesystem>

export module mythology.sdl.application_2;

namespace Mythology::SDL
{
    export void run(
        nlohmann::json const& pipeline_json,
        std::filesystem::path const& pipeline_json_parent_path,
        std::filesystem::path const& gltf_file_path
    ) noexcept;
}
