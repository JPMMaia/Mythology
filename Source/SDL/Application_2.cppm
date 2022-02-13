module;

#include <nlohmann/json.hpp>

#include <filesystem>
#include <span>
#include <string>
#include <unordered_map>

export module mythology.sdl.application_2;

namespace Mythology::SDL
{
    export void run(
        std::pmr::unordered_map<std::pmr::string, std::filesystem::path> const& render_pipelines_configurations,
        std::filesystem::path const& gltf_file_path,
        std::span<std::filesystem::path const> addon_paths
    );
}
