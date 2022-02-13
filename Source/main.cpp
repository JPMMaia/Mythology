#include <docopt.h>
#include <nlohmann/json.hpp>

#include <filesystem>
#include <fstream>
#include <string>
#include <unordered_map>

import mythology.sdl.application_2;
//import mythology.windowless;

namespace
{
    bool matches(char const* const lhs, char const* const rhs) noexcept
    {
        return std::strcmp(lhs, rhs) == 0;
    }

    nlohmann::json read_json_from_file(std::filesystem::path const& path) noexcept
    {
        std::ifstream input_stream{ path };
        assert(input_stream.good());

        nlohmann::json json{};
        input_stream >> json;

        return json;
    }

    std::pmr::unordered_map<std::pmr::string, std::filesystem::path> parse_render_pipelines(
        std::map<std::string, docopt::value> const& arguments
    )
    {
        auto const parse_pipeline_argument = [](std::string const& argument) -> std::pair<std::pmr::string, std::filesystem::path>
        {
            auto const equal_location = std::find(argument.begin(), argument.end(), '=');

            if (equal_location != argument.end())
            {
                std::pmr::string const name{ argument.begin(), equal_location };
                std::pmr::string const path_string{ equal_location + 1, argument.end() };
                std::filesystem::path const path{ path_string };

                return std::make_pair(name, path);
            }
            else
            {
                return {};
            }
        };

        docopt::value const& pipelines_argument_value = arguments.at("--pipeline");

        if (pipelines_argument_value.isStringList())
        {
            std::vector<std::string> const& pipelines_arguments = pipelines_argument_value.asStringList();

            std::pmr::unordered_map<std::pmr::string, std::filesystem::path> pipeline_paths;

            for (std::string const& pipeline_argument : pipelines_arguments)
            {
                pipeline_paths.insert(
                    parse_pipeline_argument(pipeline_argument)
                );
            }

            return pipeline_paths;
        }
        else
        {
            return {};
        }
    }

    std::filesystem::path parse_gltf_argument(
        std::map<std::string, docopt::value> const& arguments
    )
    {
        if (arguments.contains("--gltf"))
        {
            return std::filesystem::path{ arguments.at("--gltf").asString() };
        }
        else
        {
            return {};
        }
    }

    std::pmr::vector<std::filesystem::path> parse_addon_paths(
        std::map<std::string, docopt::value> const& arguments
    )
    {
        if (arguments.contains("--addon"))
        {
            std::vector<std::string> const string_list = arguments.at("--addon").asStringList();

            std::pmr::vector<std::filesystem::path> addon_paths;
            addon_paths.reserve(string_list.size());

            for (std::string const& string : string_list)
            {
                addon_paths.push_back(std::filesystem::path{ string });
            }

            return addon_paths;
        }
        else
        {
            return {};
        }
    }
}

constexpr const char* c_usage =
R"(Mythology.

    Usage:
        mythology render --pipeline=<pipeline_name>=<pipeline_json_file>... [--gltf=<gltf_json_file>] --output=<output_file>
        mythology window --pipeline=<pipeline_name>=<pipeline_json_file>... [--gltf=<gltf_json_file>] [--addon=<addon_file>]...
        mythology [--help]
        mythology --version

    Options:
        -h --help       Print this message.
        --version       Print version.

)";

int main(int const argc, const char* const* const argv) noexcept
{
    std::map<std::string, docopt::value> const arguments = docopt::docopt(
        c_usage,
        { argv + 1, argv + argc },
        true,
        "Mythology 0.1"
    );

    std::pmr::unordered_map<std::pmr::string, std::filesystem::path> const render_pipelines =
        parse_render_pipelines(arguments);

    if (arguments.at("render").asBool())
    {
        //Mythology::Windowless::render_frame("output.ppm");
    }
    else if (arguments.at("window").asBool())
    {
        std::filesystem::path const gltf_path = parse_gltf_argument(arguments);
        std::pmr::vector<std::filesystem::path> const addon_paths = parse_addon_paths(arguments);
        Mythology::SDL::run(render_pipelines, gltf_path, addon_paths);
    }

    return 0;
}