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
                {"bufferView", 1},
                {"byteOffset", 4},
                {"normalized", true},
                {"componentType", 5126},
                {"count", 24},
                {"max", {1, 1, 1}},
                {"min", {-1, -1, -1}},
                {"type", "VEC3"},
                {"name", "Accessor A"},
            };

            Accessor const expected
            {
                .buffer_view_index = 1,
                .byte_offset = 4,
                .normalized = true,
                .component_type = Component_type::Float,
                .count = 24,
                .type = Accessor::Type::Vector3,
                .max = Vector3f{1.0f, 1.0f, 1.0f},
                .min = Vector3f{-1.0f, -1.0f, -1.0f},
                .name = "Accessor A",
            };

            Accessor const actual = Maia::glTF::accessor_from_json(json, {});
            
            CHECK(expected == actual);
        }
    }

    TEST_CASE("Create buffer from JSON")
    {
        {
            nlohmann::json const json =
            {
                {"uri", "untitled.bin"},
                {"byteLength", 1024},
                {"name", "Buffer A"},
            };

            Buffer const expected
            {
                .uri = "untitled.bin",
                .byte_length = 1024,
                .name = "Buffer A",
            };

            Buffer const actual = Maia::glTF::buffer_from_json(json, {});
            
            CHECK(expected == actual);
        }
    }

    TEST_CASE("Create buffer view from JSON")
    {
        {
            nlohmann::json const json =
            {
                {"buffer", 1},
                {"byteOffset", 576},
                {"byteLength", 384},
                {"name", "Buffer View A"},
            };

            Buffer_view const expected
            {
                .buffer_index = 1,
                .byte_offset = 576,
                .byte_length = 384,
                .name = "Buffer View A",
            };

            Buffer_view const actual = Maia::glTF::buffer_view_from_json(json, {});
            
            CHECK(expected == actual);
        }
    }

    TEST_CASE("Create PBR metallic roughness from JSON")
    {
        {
            nlohmann::json const json =
            {
                {"baseColorFactor", {0.5f, 0.2f, 0.7f, 1.0f}},
                {"metallicFactor", 0.5f},
                {"roughnessFactor", 0.4f},
            };

            Pbr_metallic_roughness const expected
            {
                .base_color_factor = Vector4f{0.5f, 0.2f, 0.7f, 1.0f},
                .metallic_factor = 0.5f,
                .roughness_factor = 0.4f,
            };

            Pbr_metallic_roughness const actual = Maia::glTF::pbr_metallic_roughness_from_json(json);
            
            CHECK(expected == actual);
        }
    }

    TEST_CASE("Create material from JSON")
    {
        {
            nlohmann::json const json =
            {
                {
                    "pbrMetallicRoughness",
                    {
                        {"baseColorFactor", {0.5f, 0.2f, 0.7f, 1.0f}},
                        {"metallicFactor", 0.5f},
                        {"roughnessFactor", 0.4f},
                    }
                },
                {"emissiveFactor", {0.3f, 0.2f, 0.5f}},
                {"alphaMode", "MASK"},
                {"alphaCutoff", 0.6f},
                {"doubleSided", true},
                {"name", "Material A"},
            };

            Material const expected
            {
                .pbr_metallic_roughness =
                {
                    .base_color_factor = {0.5f, 0.2f, 0.7f, 1.0f},
                    .metallic_factor = 0.5f,
                    .roughness_factor = 0.4f,
                },
                .emissive_factor = {0.3f, 0.2f, 0.5f},
                .alpha_mode = Alpha_mode::Mask,
                .alpha_cutoff = 0.6f,
                .double_sided = true,
                .name = "Material A",
            };

            Material const actual = Maia::glTF::material_from_json(json, {});
            
            CHECK(expected == actual);
        }
    }
}
