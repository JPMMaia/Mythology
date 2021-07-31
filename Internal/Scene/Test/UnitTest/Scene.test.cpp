#include <catch2/catch.hpp>

#include <array>
#include <memory_resource>
#include <span>
#include <vector>

import maia.scene;

namespace Maia::ECS::Test
{
    using namespace Maia::Scene;

    namespace
    {
        template <typename T>
        std::pmr::vector<std::byte> to_raw_data(std::span<T const> const input)
        {
            std::pmr::vector<std::byte> output;
            output.resize(input.size_bytes());

            std::memcpy(output.data(), input.data(), output.size());

            return output;
        }

        template <typename T>
        std::pmr::vector<std::byte> to_raw_data(std::pmr::vector<T> const& input)
        {
            std::span<T const> const span{ input.data(), input.size() };

            return to_raw_data(span);
        }
    }

    TEST_CASE("Read unstrided position accessor")
    {
        std::pmr::vector<Vector3f> const buffer_data
        {
            Vector3f{.x = 1.0f, .y = 2.0f, .z = 3.0f},
            Vector3f{.x = 4.0f, .y = 5.0f, .z = 6.0f},
            Vector3f{.x = 7.0f, .y = 8.0f, .z = 9.0f},
            Vector3f{.x = 10.0f, .y = 11.0f, .z = 12.0f},
        };

        std::pmr::vector<std::byte> const buffer_raw_data = to_raw_data(buffer_data);

        Buffer_view const buffer_view
        {
            .buffer_index = 0,
            .byte_offset = 2 * sizeof(float),
            .byte_length = 1 * sizeof(float) + 3 * sizeof(Vector3f),
        };

        Accessor const accessor
        {
            .buffer_view_index = 0,
            .byte_offset = 1 * sizeof(float),
            .normalized = false,
            .component_type = Component_type::Float,
            .count = 3,
            .type = Accessor::Type::Vector3,
        };

        std::pmr::vector<Vector3f> const actual_data = read_position_accessor_data(
            accessor,
            { &buffer_view , 1 },
            { &buffer_raw_data , 1 },
            {},
            {}
        );

        std::pmr::vector<Vector3f> const expected_data
        {
            Vector3f{.x = 4.0f, .y = 5.0f, .z = 6.0f, },
            Vector3f{.x = 7.0f, .y = 8.0f, .z = 9.0f, },
            Vector3f{.x = 10.0f, .y = 11.0f, .z = 12.0f, },
        };

        CHECK(expected_data == actual_data);
    }

    TEST_CASE("Read strided position accessor")
    {
        struct Vertex
        {
            Vector3f normal = {};
            Vector3f position = {};
            Vector2f texture_coordinates = {};
        };

        std::pmr::vector<Vertex> const buffer_data
        {
            Vertex
            {
                .normal = Vector3f{.x = -1.0f, .y = 1.0f, .z = -1.0f, },
                .position = Vector3f{.x = 1.0f, .y = 2.0f, .z = 3.0f, },
                .texture_coordinates = Vector2f{.x = 1.0f, .y = 0.5f, },
            },
            Vertex
            {
                .normal = Vector3f{.x = 1.0f, .y = -0.5f, .z = -1.0f, },
                .position = Vector3f{.x = 3.0f, .y = 1.0f, .z = 2.0f, },
                .texture_coordinates = Vector2f{.x = 0.5f, .y = 0.5f, },
            },
            Vertex
            {
                .normal = Vector3f{.x = -1.0f, .y = 0.0f, .z = -1.0f, },
                .position = Vector3f{.x = 2.0f, .y = 2.0f, .z = 3.0f, },
                .texture_coordinates = Vector2f{.x = -1.0f, .y = 0.5f, },
            },
            Vertex
            {
                .normal = Vector3f{.x = -1.0f, .y = 2.0f, .z = -3.0f, },
                .position = Vector3f{.x = 5.0f, .y = 2.0f, .z = -3.0f, },
                .texture_coordinates = Vector2f{.x = 1.0f, .y = 0.25f, },
            }
        };

        std::pmr::vector<std::byte> const buffer_raw_data = to_raw_data(buffer_data);

        Buffer_view const buffer_view
        {
            .buffer_index = 0,
            .byte_offset = 1 * sizeof(Vertex),
            .byte_length = 3 * sizeof(Vertex),
            .byte_stride = sizeof(Vertex),
        };

        Accessor const accessor
        {
            .buffer_view_index = 0,
            .byte_offset = 1 * sizeof(Vector3f),
            .normalized = false,
            .component_type = Component_type::Float,
            .count = 3,
            .type = Accessor::Type::Vector3,
        };

        std::pmr::vector<Vector3f> const actual_data = read_position_accessor_data(
            accessor,
            { &buffer_view , 1 },
            { &buffer_raw_data , 1 },
            {},
            {}
        );

        std::pmr::vector<Vector3f> const expected_data
        {
            Vector3f{.x = 3.0f, .y = 1.0f, .z = 2.0f, },
            Vector3f{.x = 2.0f, .y = 2.0f, .z = 3.0f, },
            Vector3f{.x = 5.0f, .y = 2.0f, .z = -3.0f, },
        };

        CHECK(expected_data == actual_data);
    }

    TEST_CASE("Read accessor sparse data without buffer view")
    {
        std::pmr::vector<std::uint8_t> const sparse_indices
        {
            0,
            1,
            2,
            4,
        };

        std::pmr::vector<Vector3f> const sparse_values
        {
            Vector3f{.x = 4.0f, .y = 5.0f, .z = 2.5f},
            Vector3f{.x = 1.0f, .y = -1.0f, .z = 0.5f},
            Vector3f{.x = 5.0f, .y = -2.0f, .z = 3.0f},
        };

        std::pmr::vector<std::byte> const sparse_indices_raw_data = to_raw_data(sparse_indices);
        std::pmr::vector<std::byte> const sparse_values_raw_data = to_raw_data(sparse_values);
        std::array<std::pmr::vector<std::byte>, 2> const buffers_data
        {
            sparse_indices_raw_data, sparse_values_raw_data
        };

        std::array<Buffer_view, 2> const buffer_views
        {
            Buffer_view
            {
                .buffer_index = 0,
                .byte_offset = 1,
                .byte_length = 3,
            },
            Buffer_view
            {
                .buffer_index = 1,
                .byte_offset = 4,
                .byte_length = 32,
            },
        };

        Accessor const accessor
        {
            .component_type = Component_type::Float,
            .count = 6,
            .type = Accessor::Type::Vector3,
            .sparse = Sparse
            {
                .count = 2,
                .indices =
                {
                    .buffer_view_index = 0,
                    .byte_offset = 1,
                    .component_type = Component_type::Unsigned_byte,
                },
                .values =
                {
                    .buffer_view_index = 1,
                    .byte_offset = 8,
                },
            },
        };

        std::pmr::vector<Vector3f> const actual_data = read_position_accessor_data(
            accessor,
            buffer_views,
            buffers_data,
            {},
            {}
        );

        std::pmr::vector<Vector3f> const expected_data
        {
            Vector3f{.x = 0.0f, .y = 0.0f, .z = 0.0f},
            Vector3f{.x = 0.0f, .y = 0.0f, .z = 0.0f},
            Vector3f{.x = 1.0f, .y = -1.0f, .z = 0.5f},
            Vector3f{.x = 0.0f, .y = 0.0f, .z = 0.0f},
            Vector3f{.x = 5.0f, .y = -2.0f, .z = 3.0f},
            Vector3f{.x = 0.0f, .y = 0.0f, .z = 0.0f},
        };

        CHECK(expected_data == actual_data);
    }

    TEST_CASE("Read accessor sparse data with buffer view")
    {
        std::pmr::vector<Vector3f> const initial_values
        {
            Vector3f{.x = 1.0f, .y = -3.0f, .z = 8.0f},
            Vector3f{.x = 5.0f, .y = -2.0f, .z = 3.0f},
            Vector3f{.x = 3.0f, .y = -6.0f, .z = 5.0f},
            Vector3f{.x = 1.0f, .y = -6.0f, .z = 0.5f},
            Vector3f{.x = 5.0f, .y = 9.0f, .z = -2.0f},
            Vector3f{.x = 3.0f, .y = 1.0f, .z = 2.0f},
            Vector3f{.x = 6.0f, .y = -4.0f, .z = 5.0f},
        };

        std::pmr::vector<std::uint8_t> const sparse_indices
        {
            0,
            1,
            2,
            4,
        };

        std::pmr::vector<Vector3f> const sparse_values
        {
            Vector3f{.x = 4.0f, .y = 5.0f, .z = 2.5f},
            Vector3f{.x = 1.0f, .y = -1.0f, .z = 0.5f},
            Vector3f{.x = 5.0f, .y = -2.0f, .z = 3.0f},
        };

        std::pmr::vector<std::byte> const initial_values_raw_data = to_raw_data(initial_values);
        std::pmr::vector<std::byte> const sparse_indices_raw_data = to_raw_data(sparse_indices);
        std::pmr::vector<std::byte> const sparse_values_raw_data = to_raw_data(sparse_values);
        std::array<std::pmr::vector<std::byte>, 3> const buffers_data
        {
            initial_values_raw_data, sparse_indices_raw_data, sparse_values_raw_data
        };

        std::array<Buffer_view, 3> const buffer_views
        {
            Buffer_view
            {
                .buffer_index = 0,
                .byte_offset = 8,
                .byte_length = 64,
            },
            Buffer_view
            {
                .buffer_index = 1,
                .byte_offset = 1,
                .byte_length = 3,
            },
            Buffer_view
            {
                .buffer_index = 2,
                .byte_offset = 4,
                .byte_length = 32,
            },
        };

        Accessor const accessor
        {
            .buffer_view_index = 0,
            .byte_offset = 4,
            .component_type = Component_type::Float,
            .count = 6,
            .type = Accessor::Type::Vector3,
            .sparse = Sparse
            {
                .count = 2,
                .indices =
                {
                    .buffer_view_index = 1,
                    .byte_offset = 1,
                    .component_type = Component_type::Unsigned_byte,
                },
                .values =
                {
                    .buffer_view_index = 2,
                    .byte_offset = 8,
                },
            },
        };

        std::pmr::vector<Vector3f> const actual_data = read_position_accessor_data(
            accessor,
            buffer_views,
            buffers_data,
            {},
            {}
        );

        std::pmr::vector<Vector3f> const expected_data
        {
            Vector3f{.x = 5.0f, .y = -2.0f, .z = 3.0f},
            Vector3f{.x = 3.0f, .y = -6.0f, .z = 5.0f},
            Vector3f{.x = 1.0f, .y = -1.0f, .z = 0.5f},
            Vector3f{.x = 5.0f, .y = 9.0f, .z = -2.0f},
            Vector3f{.x = 5.0f, .y = -2.0f, .z = 3.0f},
            Vector3f{.x = 6.0f, .y = -4.0f, .z = 5.0f},
        };

        CHECK(expected_data == actual_data);
    }
}
