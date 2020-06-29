export module mythology.sdl.application;

import <nlohmann/json.hpp>;

import <filesystem>;

namespace Mythology::SDL
{
    export void run(
        nlohmann::json const& pipeline_json,
        std::filesystem::path const& pipeline_json_parent_path,
        std::filesystem::path const& gltf_file_path
    ) noexcept;
}
