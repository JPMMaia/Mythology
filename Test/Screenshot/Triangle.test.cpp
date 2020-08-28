import mythology.windowless;

import <catch2/catch.hpp>;
import <nlohmann/json.hpp>;

import <cstddef>;
import <filesystem>;
import <fstream>;
import <memory_resource>;
import <vector>;

namespace
{
	nlohmann::json read_json_from_file(std::filesystem::path const& path) noexcept
    {
        std::ifstream input_stream{path};
        assert(input_stream.good());

        nlohmann::json json{};
        input_stream >> json;

        return json;
    }
}

namespace Mythology::Windowless::Test
{
	SCENARIO("Render a triangle")
	{
		std::filesystem::path const pipeline_json_file_path = "pipeline.json";
		nlohmann::json const pipeline_json = read_json_from_file(pipeline_json_file_path);
		
		std::filesystem::path const gltf_file_path = "triangle.json";

		std::pmr::vector<std::byte> image_data = 
			Mythology::Windowless::render_frame(
				pipeline_json,
				pipeline_json_file_path.parent_path(),
				gltf_file_path,
				{},
				{}
			);

		//REQUIRE(image_data.size() > 0);
		//REQUIRE(image_data[0] == 0.0f);
    }
}
