#include <catch2/catch.hpp>

#include <nlohmann/json.hpp>

#include <optional>

import maia.glTF;
import maia.scene;

namespace Maia::ECS::Test
{
    using namespace Maia::Scene;

    TEST_CASE("Create accessor from JSON")
    {
        {
            nlohmann::json const json =
            {
                {"bufferView", 0},
                {"componentType", 5126},
                {"count", 24},
                {"max", {1, 1, 1}},
                {"min", {-1, -1, -1}},
                {"type", "VEC3"}
            };

            Accessor const expected
            {
                .buffer_view_index = 0,
                .byte_offset = 0,
                .normalized = false,
                .component_type = Component_type::Float,
                .count = 24,
                .type = Accessor::Type::Vector3,
                .max = Vector3f{1.0f, 1.0f, 1.0f},
                .min = Vector3f{-1.0f, -1.0f, -1.0f},
                .name = std::nullopt,
            };

            Accessor const actual = Maia::glTF::accessor_from_json(json);
            
            CHECK(expected == actual);
        }
    }
}
