import mythology.windowless;

import mythology.image;

import <catch2/catch.hpp>;
import <nlohmann/json.hpp>;

import <array>;
import <cstddef>;
import <filesystem>;
import <fstream>;
import <memory_resource>;
import <span>;
import <vector>;

namespace
{
	nlohmann::json read_json_from_file(std::filesystem::path const& path) noexcept
    {
		assert(std::filesystem::exists(path));

        std::ifstream input_stream{path};
        assert(input_stream.good());

        nlohmann::json json{};
        input_stream >> json;

        return json;
    }
}

namespace Mythology::Windowless::Test
{
	SCENARIO("Render a frame with a white triangle on black background using only the pipeline")
	{
		std::filesystem::path const working_directory = std::filesystem::current_path();
		std::filesystem::path const pipeline_json_file_path = working_directory / "test_assets/white_triangle_pipeline.json";
		REQUIRE(std::filesystem::exists(pipeline_json_file_path));

		nlohmann::json const pipeline_json = read_json_from_file(pipeline_json_file_path);

		Mythology::Windowless::Frame_dimensions constexpr frame_dimensions{9, 9};
		std::uint8_t constexpr pixel_size_in_bytes = 4;

		std::pmr::vector<std::byte> image_data = 
			Mythology::Windowless::render_frame(
				frame_dimensions,
				pipeline_json,
				pipeline_json_file_path.parent_path(),
				{},
				{},
				{}
			);

		std::uint64_t constexpr expected_image_data_size = pixel_size_in_bytes*frame_dimensions.width*frame_dimensions.height;
		REQUIRE(image_data.size() == expected_image_data_size);

		std::array<std::uint8_t, expected_image_data_size> const uint8_image_data = [&image_data]{
			std::array<std::uint8_t, expected_image_data_size> uint8_image_data;
			std::memcpy(uint8_image_data.data(), image_data.data(), uint8_image_data.size());
			return uint8_image_data;
		}();

		Mythology::Image::Image const expected_image = Mythology::Image::read_image(
			working_directory / "test_assets/white_triangle_expected_output.png",
			{}
		);

		CHECK(expected_image.data.size() == image_data.size());
		CHECK(expected_image.data == image_data);

		{
			Mythology::Image::Image_data_layout image_data_layout
			{
				.width = frame_dimensions.width,
				.height = frame_dimensions.height,
				.channel_count = 4,
				.bytes_per_channel = 1,
			};

			Mythology::Image::write_png_image(
				working_directory / "test_assets/white_triangle_actual_output.png",
				image_data,
				image_data_layout
			);
		}
    }
}
