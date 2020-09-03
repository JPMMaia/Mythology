export module mythology.image;

import <cstddef>;
import <cstdint>;
import <filesystem>;
import <memory_resource>;
import <span>;
import <vector>;

namespace Mythology::Image
{
    export struct Image_data_layout
    {
        std::uint64_t width;
        std::uint64_t height;
        std::uint8_t channel_count;
        std::uint8_t bytes_per_channel;
    };

    export bool operator==(Image_data_layout const& lhs, Image_data_layout const& rhs) noexcept;
    export bool operator!=(Image_data_layout const& lhs, Image_data_layout const& rhs) noexcept;

    export struct Image
    {
        std::pmr::vector<std::byte> data;
        Image_data_layout layout;
    };

    export Image read_image(
        std::filesystem::path const& image_path,
        std::pmr::polymorphic_allocator<> const& output_allocator
    ) noexcept;

    export void write_png_image(
        std::filesystem::path const& image_path,
        std::span<std::byte const> image_data_to_write,
        Image_data_layout image_data_layout
    ) noexcept;
}
