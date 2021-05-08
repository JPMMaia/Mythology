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

    TEST_CASE("Create primitive from JSON")
    {
        {
            nlohmann::json const json =
            {
                {
                    "attributes",
                    {
                        {"POSITION", 0},
                        {"NORMAL", 1},
                        {"TANGENT", 2},
                        {"TEXCOORD_0", 3},
                        {"TEXCOORD_1", 4},
                        {"COLOR_0", 5},
                        {"JOINTS_0", 6},
                        {"WEIGHTS_0", 7},
                    }
                },
                {"indices", 8},
                {"mode", 5},
                {"material", 1},
            };

            Primitive const expected
            {
                .attributes =
                {
                    {{Attribute::Type::Position, 0}, 0},
                    {{Attribute::Type::Normal, 0}, 1},
                    {{Attribute::Type::Tangent, 0}, 2},
                    {{Attribute::Type::Texture_coordinate, 0},3},
                    {{Attribute::Type::Texture_coordinate, 1}, 4},
                    {{Attribute::Type::Color, 0}, 5},
                    {{Attribute::Type::Joints, 0}, 6},
                    {{Attribute::Type::Weights, 0}, 7},
                },
                .indices_index = 8,
                .mode = Primitive::Mode::Triangle_strip,
                .material_index = 1,
            };

            Primitive const actual = Maia::glTF::primitive_from_json(json, {});
            
            CHECK(expected == actual);
        }
    }

    TEST_CASE("Create mesh from JSON")
    {
        {
            nlohmann::json const json =
            {
                {"name", "Cube",},
                {
                    "primitives",
                    {
                        {
                            {
                                "attributes",
                                {
                                    {"POSITION", 0},
                                    {"NORMAL", 1},
                                }
                            },
                            {"indices", 2},
                            {"material", 0},
                        },
                        {
                            {
                                "attributes",
                                {
                                    {"POSITION", 3},
                                    {"NORMAL", 4},
                                }
                            },
                            {"indices", 5},
                            {"material", 1},
                        },
                    }
                },
            };

            Mesh const expected
            {
                .primitives =
                {
                    Primitive
                    {
                        .attributes =
                        {
                            {{Attribute::Type::Position, 0}, 0},
                            {{Attribute::Type::Normal, 0}, 1},
                        },
                        .indices_index = 2,
                        .material_index = 0,
                    },
                    Primitive
                    {
                        .attributes =
                        {
                            {{Attribute::Type::Position, 0}, 3},
                            {{Attribute::Type::Normal, 0}, 4},
                        },
                        .indices_index = 5,
                        .material_index = 1,
                    },
                },
                .name = "Cube",
            };

            Mesh const actual = Maia::glTF::mesh_from_json(json, {});
            
            CHECK(expected == actual);
        }
    }

    TEST_CASE("Create camera from JSON")
    {
        SECTION("Orthographic")
        {
            nlohmann::json const json =
            {
                {"name", "Orthographic camera"},
                {"type", "orthographic"},
                {
                    "orthographic",
                    {
                        {"xmag", 1.5f},
                        {"ymag", 0.5f},
                        {"znear", 0.25f},
                        {"zfar", 100.0f},
                    }
                },
            };

            Camera const expected
            {
                .type = Camera::Type::Orthographic,
                .projection = Camera::Orthographic
                {
                    .horizontal_magnification = 1.5f,
                    .vertical_magnification = 0.5f,
                    .near_z = 0.25f,
                    .far_z = 100.0f,
                },
                .name = "Orthographic camera",
            };

            Camera const actual = Maia::glTF::camera_from_json(json, {});
            
            CHECK(expected == actual);
        }

        SECTION("Perspective")
        {
            nlohmann::json const json =
            {
                {"name", "Camera"},
                {
                    "perspective",
                    {
                        {"aspectRatio", 1},
                        {"yfov", 0.5},
                        {"znear", 0.25},
                        {"zfar", 100},
                    },
                },
                {"type", "perspective"},
            };

            Camera const expected
            {
                .type = Camera::Type::Perspective,
                .projection = Camera::Perspective
                {
                    .aspect_ratio = 1.0f,
                    .vertical_field_of_view = 0.5f,
                    .near_z = 0.25f,
                    .far_z = 100.0f,
                },
                .name = "Camera",
            };

            Camera const actual = Maia::glTF::camera_from_json(json, {});
            
            CHECK(expected == actual);
        }
    }

    TEST_CASE("Create node from JSON")
    {
        SECTION("Camera")
        {
            nlohmann::json const json =
            {
                {"camera", 1},
                {"name", "Camera"},
            };

            Node const expected
            {
                .camera_index = 1,
                .name = "Camera",
            };

            Node const actual = Maia::glTF::node_from_json(json, {});
            
            CHECK(expected == actual);
        }

        SECTION("Mesh")
        {
            nlohmann::json const json =
            {
                {"mesh", 2},
                {"name", "Cube"},
            };

            Node const expected
            {
                .mesh_index = 2,
                .name = "Cube",
            };

            Node const actual = Maia::glTF::node_from_json(json, {});
            
            CHECK(expected == actual);
        }

        SECTION("Matrix translation and scale")
        {
            nlohmann::json const json =
            {
                {
                    "matrix",
                    {
                        2.0f, 0.0f, 0.0f, 0.0f,
                        0.0f, 1.0f, 0.0f, 0.0f,
                        0.0f, 0.0f, 3.0f, 0.0f,
                        2.0f, -3.0f, 1.0f, 1.0f,
                    }
                },
                {"name", "Cube"},
            };

            Node const expected
            {
                .rotation = {0.0f, 0.0f, 0.0f, 1.0f},
                .scale = {2.0f, 1.0f, 3.0f},
                .translation = {2.0f, -3.0f, 1.0f},
                .name = "Cube",
            };

            Node const actual = Maia::glTF::node_from_json(json, {});
            
            CHECK(expected == actual);
        }

        SECTION("Matrix rotation")
        {
            nlohmann::json const json =
            {
                {
                    "matrix",
                    {
                        1.0f, 0.0f, 0.0f, 0.0f,
                        0.0f, -1.0f, 0.0f, 0.0f,
                        0.0f, 0.0f, -1.0f, 0.0f,
                        0.0f, 0.0f, 0.0f, 1.0f,
                    }
                },
                {"name", "Cube"},
            };

            Node const expected
            {
                .rotation = {1.0f, 0.0f, 0.0f, 0.0f},
                .scale = {1.0f, 1.0f, 1.0f},
                .translation = {0.0f, 0.0f, 0.0f},
                .name = "Cube",
            };

            Node const actual = Maia::glTF::node_from_json(json, {});
            
            CHECK(expected == actual);
        }

        SECTION("Matrix mix rotation and scale")
        {
            nlohmann::json const json =
            {
                {
                    "matrix",
                    {
                        2.0f, 0.0f, 0.0f, 0.0f,
                        0.0f, -3.0f, 0.0f, 0.0f,
                        0.0f, 0.0f, -4.0f, 0.0f,
                        0.0f, 0.0f, 0.0f, 1.0f,
                    }
                },
                {"name", "Cube"},
            };

            Node const expected
            {
                .rotation = {1.0f, 0.0f, 0.0f, 0.0f},
                .scale = {2.0f, 3.0f, 4.0f},
                .translation = {0.0f, 0.0f, 0.0f},
                .name = "Cube",
            };

            Node const actual = Maia::glTF::node_from_json(json, {});
            
            CHECK(expected == actual);
        }

        SECTION("Transform")
        {
            nlohmann::json const json =
            {
                {"rotation", {0.5f, 0.0f, 0.0f, 0.5f}},
                {"scale", {3.0f, 0.5f, 2.0f}},
                {"translation", {-1.0f, 2.0f, -3.0f}},
                {"name", "Cube"},
            };

            Node const expected
            {
                .rotation = {0.5f, 0.0f, 0.0f, 0.5f},
                .scale = {3.0f, 0.5f, 2.0f},
                .translation = {-1.0f, 2.0f, -3.0f},
                .name = "Cube",
            };

            Node const actual = Maia::glTF::node_from_json(json, {});
            
            CHECK(expected == actual);
        }

        SECTION("Children")
        {
            nlohmann::json const json =
            {
                {
                    "children",
                    {
                        0, 1, 3, 5
                    }
                },
            };

            Node const expected
            {
                .child_indices = {0, 1, 3, 5},
            };

            Node const actual = Maia::glTF::node_from_json(json, {});
            
            CHECK(expected == actual);
        }
    }

    TEST_CASE("Create scene from JSON")
    {
        {
            nlohmann::json const json =
            {
                {"name", "Scene"},
                {
                    "nodes",
                    {
                        0, 1, 3
                    }
                },
            };

            Maia::Scene::Scene const expected
            {
                .nodes = {{0, 1, 3}},
                .name = "Scene",
            };

            Maia::Scene::Scene const actual = Maia::glTF::scene_from_json(json, {});
            
            CHECK(expected == actual);
        }
    }
}
