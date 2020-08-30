import mythology.windowless;

import <catch2/catch.hpp>;
import <nlohmann/json.hpp>;

import <array>;
import <cstddef>;
import <filesystem>;
import <fstream>;
import <memory_resource>;
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
	SCENARIO("Render a frame with a clear color")
	{
		std::filesystem::path const pipeline_json_file_path = "../share/clear_blue_pipeline.json";
		REQUIRE(std::filesystem::exists(pipeline_json_file_path));

		nlohmann::json const pipeline_json = read_json_from_file(pipeline_json_file_path);

		std::pmr::vector<std::byte> image_data = 
			Mythology::Windowless::render_frame(
				Mythology::Windowless::Frame_dimensions{1, 1},
				pipeline_json,
				pipeline_json_file_path.parent_path(),
				{},
				{},
				{}
			);

		REQUIRE(image_data.size() == 4);

		std::array<std::uint8_t, 4> const uint8_image_data = [&image_data]{
			std::array<std::uint8_t, 4> uint8_image_data;
			std::memcpy(uint8_image_data.data(), image_data.data(), uint8_image_data.size());
			return uint8_image_data;
		}();
		
		CHECK(uint8_image_data[0] == 10);
		CHECK(uint8_image_data[1] == 28);
		CHECK(uint8_image_data[2] == 192);
		CHECK(uint8_image_data[3] == 255);
    }
}
