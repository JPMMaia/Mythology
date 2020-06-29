//#include <docopt.h>

import mythology.sdl.application;
import mythology.windowless;

import <nlohmann/json.hpp>;

import <filesystem>;
import <fstream>;

namespace
{
    bool matches(char const* const lhs, char const* const rhs) noexcept
    {
        return std::strcmp(lhs, rhs) == 0;
    }

    nlohmann::json read_json_from_file(std::filesystem::path const& path) noexcept
    {
        std::ifstream input_stream{path};

        nlohmann::json json{};
        input_stream >> json;

        return json;
    }
}

constexpr const char* c_usage = 
R"(Mythology.

    Usage:
        mythology render --pipeline <pipeline_json_file> --output <output_file>
        mythology window --pipeline <pipeline_json_file>
        mythology [--help]
        mythology --version

    Options:
        -h --help       Print this message.
        -o --output     Specify output image file which will contain the render result.
        --version       Print version.

)";

int main(int const argc, const char* const* const argv) noexcept
{
    /*std::map<std::string, docopt::value> const args = docopt::docopt(
        c_usage,
        {argv + 1, argv + argc},
        true,
        "Mythology 0.1"
    );*/

    //Mythology::Windowless::render_frame("output.ppm");

    if (argc >= 6)
    {
        if (matches(argv[1], "window"))
        {
            if (matches(argv[2], "--pipeline"))
            {
                if (matches(argv[4], "--gltf"))
                {
                    std::filesystem::path const pipeline_json_file_path = argv[3];
                    std::filesystem::path const gltf_file_path = argv[5];

                    nlohmann::json const pipeline_json = read_json_from_file(pipeline_json_file_path);
                    Mythology::SDL::run(pipeline_json, pipeline_json_file_path.parent_path(), gltf_file_path);
                }
            }
        }
    }

    return 0;
}