module;

#define CATCH_CONFIG_RUNNER
#include <catch2/catch.hpp>

module catch2.runner;

int run_catch2(int const argc, char const* const* const argv) noexcept
{
    return Catch::Session().run(argc, argv);
}