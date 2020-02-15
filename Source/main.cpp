//#include <docopt.h>

//import mythology.sdl.application;
import mythology.windowless;

import <filesystem>;

constexpr const char* c_usage = 
R"(Mythology.

    Usage:
        mythology render --output <output_file>
        mythology window
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

    /*for (auto const& arg : args)
    {
        std::cout << arg.first <<  arg.second << std::endl;
    }*/

    Mythology::Windowless::render_frame("output.ppm");

    return 0;
}