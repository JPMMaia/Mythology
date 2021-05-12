#define CATCH_CONFIG_RUNNER
#include <catch2/catch.hpp>

int main(int const argc, char const* const* const argv)
{
    return Catch::Session().run(argc, argv);
}
