module mythology.image;

import <stb_image.h>;
import <stb_image_write.h>;

import <array>;
import <cassert>;
import <cstddef>;
import <cstdint>;
import <cstring>;
import <filesystem>;
import <iostream>;
import <memory>;
import <memory_resource>;
import <span>;
import <vector>;

namespace Mythology::Image
{
    bool operator==(Image_data_layout const& lhs, Image_data_layout const& rhs) noexcept
    {
        return lhs.width == rhs.width
            && lhs.height == rhs.height
            && lhs.channel_count == rhs.channel_count
            && lhs.bytes_per_channel == rhs.bytes_per_channel;
    }

    bool operator!=(Image_data_layout const& lhs, Image_data_layout const& rhs) noexcept
    {
        return !(lhs == rhs);
    }

    Image read_image(
        std::filesystem::path const& image_path,
        std::pmr::polymorphic_allocator<> const& output_allocator
    ) noexcept
    {
        int width = 0;
        int height = 0;
        int channel_count = 0;

        auto const free_image = [](unsigned char* data) noexcept -> void
        {
            stbi_image_free(data);
        };

        std::unique_ptr<unsigned char, decltype(free_image)> image_data
        {
            stbi_load(image_path.c_str(), &width, &height, &channel_count, 0),
            free_image
        };
        
        if (image_data == nullptr)
        {
            std::cerr << stbi_failure_reason() << '\n';
            //assert(false);
            return {};
        }

        std::uint64_t const number_of_bytes = width * height * channel_count;
        
        std::pmr::vector<std::byte> output_image_data{output_allocator};
        output_image_data.resize(number_of_bytes);

        std::memcpy(output_image_data.data(), image_data.get(), output_image_data.size());

        Image_data_layout const image_data_layout
        {
            .width = static_cast<std::uint64_t>(width),
            .height = static_cast<std::uint64_t>(height),
            .channel_count = static_cast<std::uint8_t>(channel_count),
            .bytes_per_channel = 1,
        };

        return 
        {
            .data = std::move(output_image_data),
            .layout = image_data_layout
        };
    }

    void write_png_image(
        std::filesystem::path const& image_path,
        std::span<std::byte const> image_data_to_write,
        Image_data_layout image_data_layout
    ) noexcept
    {
        std::uint64_t const row_size_in_bytes =
            image_data_layout.width *
            image_data_layout.channel_count *
            image_data_layout.bytes_per_channel;

        int const result = stbi_write_png(
            image_path.c_str(),
            static_cast<int>(image_data_layout.width),
            static_cast<int>(image_data_layout.height),
            static_cast<int>(image_data_layout.channel_count),
            image_data_to_write.data(),
            static_cast<int>(row_size_in_bytes)
        );

        assert(result != 0);
    }
}
