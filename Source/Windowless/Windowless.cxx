export module mythology.windowless;

import <nlohmann/json.hpp>;

import <cstddef>;
import <filesystem>;
import <memory_resource>;
import <vector>;

namespace Mythology::Windowless
{
    export std::pmr::vector<std::byte> render_frame(
        nlohmann::json const& pipeline_json,
        std::filesystem::path const& pipeline_json_parent_path,
        std::filesystem::path const& gltf_file_path,
        std::pmr::polymorphic_allocator<std::byte> const& output_allocator,
        std::pmr::polymorphic_allocator<std::byte> const& temporaries_allocator
    ) noexcept;
}