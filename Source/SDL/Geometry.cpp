module;

#include <klein/klein.hpp>
#include <vulkan/vulkan.hpp>

#include <algorithm>
#include <array>
#include <cassert>
#include <execution>
#include <filesystem>
#include <format>
#include <iostream>
#include <limits>
#include <memory_resource>
#include <optional>
#include <ranges>
#include <span>
#include <vector>

module mythology.geometry;

import maia.renderer.vulkan.buffer_resources;
import maia.renderer.vulkan.upload;
import maia.scene;

using namespace Maia::Scene;

namespace Mythology
{
    template<typename T>
    struct To_vector
    {
        std::pmr::polymorphic_allocator<> const& allocator;

        std::pmr::vector<T> operator()(std::ranges::view auto view) const
        {
            return std::pmr::vector<T> { view.begin(), view.end(), this->allocator };
        }
    };

    template<typename T>
    To_vector<T> to_vector(std::pmr::polymorphic_allocator<> const& allocator)
    {
        return To_vector<T>{allocator};
    }

    template<typename T>
    auto operator|(std::ranges::view auto view, To_vector<T> const& to_vector)
    {
        return to_vector(view);
    }

    struct Geometry_buffer_views
    {
        Maia::Renderer::Vulkan::Buffer_view position = {};
        Maia::Renderer::Vulkan::Buffer_view index = {};
    };

    using Instance_buffer_view = Maia::Renderer::Vulkan::Buffer_view;

    namespace
    {
        Accessor const& get_position_accessor(
            World const& world,
            Primitive const& primitive
        ) noexcept
        {
            constexpr Attribute position_attribute
            {
                .type = Attribute::Type::Position,
                .index = 0
            };

            Index const position_accessor_index = primitive.attributes.at(position_attribute);

            return world.accessors[position_accessor_index];
        }

        Accessor const& get_index_accessor(
            World const& world,
            Primitive const& primitive
        ) noexcept
        {
            assert(primitive.indices_index.has_value());

            Index const indices_accessor_index = *primitive.indices_index;

            return world.accessors[indices_accessor_index];
        }

        Maia::Renderer::Vulkan::Buffer_view create_buffer(
            Maia::Renderer::Vulkan::Buffer_resources& buffer_resources,
            Accessor const& accessor,
            Component_type const component_type
        )
        {
            std::size_t const size_in_bytes = accessor.count * size_of(accessor.type) * size_of(component_type);

            return buffer_resources.allocate_buffer(
                size_in_bytes,
                vk::MemoryPropertyFlagBits::eDeviceLocal
            );
        };

        Component_type convert_index_accessor_component_type(Component_type const component_type) noexcept
        {
            assert(component_type == Component_type::Unsigned_byte || component_type == Component_type::Unsigned_short || component_type == Component_type::Unsigned_int);

            return component_type == Component_type::Unsigned_byte ?
                Component_type::Unsigned_short :
                component_type;
        }
    }

    std::pmr::vector<std::pmr::vector<Geometry_buffer_views>> create_geometry_buffers(
        Maia::Renderer::Vulkan::Buffer_resources& buffer_resources,
        World const& world,
        std::pmr::polymorphic_allocator<> const& output_allocator
    )
    {
        {
            auto const must_have_index_buffer_defined = [](Mesh const& mesh) -> bool
            {
                return std::all_of(
                    mesh.primitives.begin(),
                    mesh.primitives.end(),
                    [](Primitive const& primitive) -> bool {return primitive.indices_index.has_value(); }
                );
            };

            bool const result = std::all_of(
                world.meshes.begin(),
                world.meshes.end(),
                must_have_index_buffer_defined
            );

            if (!result)
            {
                throw std::runtime_error{ "Index buffer must be defined!" };
            }
        }

        auto create_geometry_buffers = [&](Mesh const& mesh) -> std::pmr::vector<Geometry_buffer_views>
        {
            auto const create_buffers = [&](Primitive const& primitive) -> Geometry_buffer_views
            {
                Accessor const& position_accessor = get_position_accessor(world, primitive);
                Maia::Renderer::Vulkan::Buffer_view const position_buffer_view = create_buffer(buffer_resources, position_accessor, position_accessor.component_type);

                Accessor const& index_accessor = get_index_accessor(world, primitive);
                Component_type const index_accessor_component_type = convert_index_accessor_component_type(index_accessor.component_type);
                Maia::Renderer::Vulkan::Buffer_view const index_buffer_view = create_buffer(buffer_resources, index_accessor, index_accessor_component_type);

                return
                {
                    .position = position_buffer_view,
                    .index = index_buffer_view,
                };
            };

            std::pmr::vector<Geometry_buffer_views> buffer_views{ output_allocator };
            buffer_views.resize(mesh.primitives.size());

            std::transform(
                mesh.primitives.begin(),
                mesh.primitives.end(),
                buffer_views.begin(),
                create_buffers
            );

            return buffer_views;
        };

        std::pmr::vector<std::pmr::vector<Geometry_buffer_views>> geometry_buffer_views{ output_allocator };
        geometry_buffer_views.resize(world.meshes.size());

        std::transform(
            world.meshes.begin(),
            world.meshes.end(),
            geometry_buffer_views.begin(),
            create_geometry_buffers
        );

        return geometry_buffer_views;
    }

    namespace
    {
        template <typename Data_type>
        void map_memory_and_copy_data(
            vk::Device const device,
            Maia::Renderer::Vulkan::Buffer_view const& destination,
            std::span<Data_type const> const data
        ) noexcept
        {
            void* const mapped_data = device.mapMemory(
                destination.memory,
                destination.offset,
                destination.size,
                vk::MemoryMapFlags{}
            );

            std::memcpy(
                mapped_data,
                data.data(),
                data.size_bytes()
            );

            device.unmapMemory(
                destination.memory
            );
        }

        template <typename Data_type>
        void map_memory_and_copy_data(
            vk::Device const device,
            Maia::Renderer::Vulkan::Buffer_view const& destination,
            std::pmr::vector<Data_type> const& data
        ) noexcept
        {
            std::span<Data_type const> const span = { data.data(), data.size() };
            map_memory_and_copy_data(device, destination, span);
        }

        vk::CommandBuffer create_one_time_submit_command_buffer(
            vk::Device const device,
            vk::CommandPool const command_pool
        ) noexcept
        {
            vk::CommandBufferAllocateInfo const allocate_info
            {
                .commandPool = command_pool,
                .level = vk::CommandBufferLevel::ePrimary,
                .commandBufferCount = 1,
            };

            std::array<std::byte, sizeof(vk::CommandBuffer)> local_storage;
            std::pmr::monotonic_buffer_resource local_storage_buffer_resource{ &local_storage, local_storage.size() };
            std::pmr::polymorphic_allocator<vk::CommandBuffer> local_storage_allocator{ &local_storage_buffer_resource };
            std::pmr::vector<vk::CommandBuffer> const command_buffers = device.allocateCommandBuffers(allocate_info, local_storage_allocator);
            vk::CommandBuffer const command_buffer = command_buffers[0];

            {
                vk::CommandBufferBeginInfo const begin_info
                {
                };

                command_buffer.begin(begin_info);
            }

            return command_buffer;
        }

        void submit_and_wait(
            vk::Device const device,
            vk::Queue const queue,
            vk::CommandPool const command_pool,
            vk::CommandBuffer const command_buffer,
            vk::AllocationCallbacks const* const allocation_callbacks
        ) noexcept
        {
            command_buffer.end();

            vk::Fence const fence = device.createFence({}, allocation_callbacks);

            vk::SubmitInfo const submit_info
            {
                .commandBufferCount = 1,
                .pCommandBuffers = &command_buffer,
            };

            queue.submit(1, &submit_info, fence);

            device.waitForFences(1, &fence, true, std::numeric_limits<std::uint64_t>::max());

            device.destroy(fence, allocation_callbacks);

            device.freeCommandBuffers(command_pool, 1, &command_buffer);
        }
    }

    template <typename Data_type>
    void upload_data_through_upload_buffer(
        vk::Device const device,
        vk::Queue const queue,
        vk::CommandPool const command_pool,
        Maia::Renderer::Vulkan::Buffer_view const& destination_buffer_view,
        std::span<Data_type const> const data_to_upload,
        Maia::Renderer::Vulkan::Upload_buffer const& upload_buffer,
        vk::AllocationCallbacks const* const allocation_callbacks
    ) noexcept
    {
        assert(data_to_upload.size() <= upload_buffer.buffer_view().size);

        std::memcpy(
            upload_buffer.mapped_data(),
            data_to_upload.data(),
            data_to_upload.size_bytes()
        );

        {
            vk::CommandBuffer const command_buffer = create_one_time_submit_command_buffer(device, command_pool);

            {
                Maia::Renderer::Vulkan::Buffer_view const upload_buffer_view = upload_buffer.buffer_view();

                vk::BufferCopy const buffer_copy
                {
                    .srcOffset = upload_buffer_view.offset,
                    .dstOffset = destination_buffer_view.offset,
                    .size = data_to_upload.size_bytes(),
                };

                command_buffer.copyBuffer(
                    upload_buffer_view.buffer,
                    destination_buffer_view.buffer,
                    1,
                    &buffer_copy
                );
            }

            submit_and_wait(device, queue, command_pool, command_buffer, allocation_callbacks);
        }
    }

    void upload_geometry_data(
        vk::PhysicalDeviceType const physical_device_type,
        vk::Device const device,
        vk::Queue const queue,
        vk::CommandPool const command_pool,
        Maia::Renderer::Vulkan::Buffer_resources& upload_buffer_resources,
        World const& world,
        std::span<std::pmr::vector<std::byte> const> const buffers_data,
        std::span<std::pmr::vector<Geometry_buffer_views> const> const geometry_buffer_views,
        vk::AllocationCallbacks const* const allocation_callbacks,
        std::pmr::polymorphic_allocator<> const& temporaries_allocator
    )
    {
        if (physical_device_type == vk::PhysicalDeviceType::eIntegratedGpu)
        {
            for (std::size_t mesh_index = 0; mesh_index < world.meshes.size(); ++mesh_index)
            {
                Mesh const& mesh = world.meshes[mesh_index];

                for (std::size_t primitive_index = 0; primitive_index < world.meshes.size(); ++primitive_index)
                {
                    Primitive const& primitive = mesh.primitives[primitive_index];

                    Geometry_buffer_views const& geometry_buffer_view =
                        geometry_buffer_views[mesh_index][primitive_index];

                    {
                        Accessor const& position_accessor = get_position_accessor(world, primitive);

                        std::pmr::vector<Maia::Scene::Vector3f> const position_data =
                            read_position_accessor_data(
                                position_accessor,
                                world.buffer_views,
                                buffers_data,
                                temporaries_allocator,
                                temporaries_allocator
                            );

                        map_memory_and_copy_data(
                            device,
                            geometry_buffer_view.position,
                            position_data
                        );
                    }

                    {
                        Accessor const& index_accessor = get_index_accessor(world, primitive);
                        Component_type const index_accessor_component_type = convert_index_accessor_component_type(index_accessor.component_type);

                        if (index_accessor_component_type == Component_type::Unsigned_short)
                        {
                            std::pmr::vector<std::uint16_t> const index_data =
                                read_indices_16_accessor_data(
                                    index_accessor,
                                    world.buffer_views,
                                    buffers_data,
                                    temporaries_allocator,
                                    temporaries_allocator
                                );

                            map_memory_and_copy_data(
                                device,
                                geometry_buffer_view.index,
                                index_data
                            );
                        }
                        else
                        {
                            assert(index_accessor_component_type == Component_type::Unsigned_int);

                            std::pmr::vector<std::uint32_t> const index_data =
                                read_indices_32_accessor_data(
                                    index_accessor,
                                    world.buffer_views,
                                    buffers_data,
                                    temporaries_allocator,
                                    temporaries_allocator
                                );

                            map_memory_and_copy_data(
                                device,
                                geometry_buffer_view.index,
                                index_data
                            );
                        }
                    }
                }
            }
        }
        else
        {
            Maia::Renderer::Vulkan::Upload_buffer const upload_buffer
            {
                device,
                upload_buffer_resources,
                256 * 1024 * 1024 // TODO get maximum size from data to upload
            };

            for (std::size_t mesh_index = 0; mesh_index < world.meshes.size(); ++mesh_index)
            {
                Mesh const& mesh = world.meshes[mesh_index];

                for (std::size_t primitive_index = 0; primitive_index < world.meshes.size(); ++primitive_index)
                {
                    Primitive const& primitive = mesh.primitives[primitive_index];

                    Geometry_buffer_views const& geometry_buffer_view =
                        geometry_buffer_views[mesh_index][primitive_index];

                    {
                        Accessor const& position_accessor = get_position_accessor(world, primitive);

                        std::pmr::vector<Maia::Scene::Vector3f> const position_data =
                            read_position_accessor_data(
                                position_accessor,
                                world.buffer_views,
                                buffers_data,
                                temporaries_allocator,
                                temporaries_allocator
                            );


                        upload_data_through_upload_buffer<Maia::Scene::Vector3f>(
                            device,
                            queue,
                            command_pool,
                            geometry_buffer_view.position,
                            position_data,
                            upload_buffer,
                            allocation_callbacks
                            );
                    }

                    {
                        Accessor const& index_accessor = get_index_accessor(world, primitive);
                        Component_type const index_accessor_component_type = convert_index_accessor_component_type(index_accessor.component_type);

                        if (index_accessor_component_type == Component_type::Unsigned_short)
                        {
                            std::pmr::vector<std::uint16_t> const index_data =
                                read_indices_16_accessor_data(
                                    index_accessor,
                                    world.buffer_views,
                                    buffers_data,
                                    temporaries_allocator,
                                    temporaries_allocator
                                );

                            upload_data_through_upload_buffer<std::uint16_t>(
                                device,
                                queue,
                                command_pool,
                                geometry_buffer_view.index,
                                index_data,
                                upload_buffer,
                                allocation_callbacks
                                );
                        }
                        else
                        {
                            assert(index_accessor_component_type == Component_type::Unsigned_int);

                            std::pmr::vector<std::uint32_t> const index_data =
                                read_indices_32_accessor_data(
                                    index_accessor,
                                    world.buffer_views,
                                    buffers_data,
                                    temporaries_allocator,
                                    temporaries_allocator
                                );

                            upload_data_through_upload_buffer<std::uint32_t>(
                                device,
                                queue,
                                command_pool,
                                geometry_buffer_view.index,
                                index_data,
                                upload_buffer,
                                allocation_callbacks
                                );
                        }
                    }
                }
            }
        }
    }

    vk::IndexType get_primitive_index_type(
        World const& world,
        Primitive const& primitive
    ) noexcept
    {
        Accessor const& index_accessor = get_index_accessor(world, primitive);
        Component_type const index_accessor_component_type = convert_index_accessor_component_type(index_accessor.component_type);

        return (index_accessor_component_type == Component_type::Unsigned_int) ?
            vk::IndexType::eUint32 :
            vk::IndexType::eUint16;
    }

    std::pmr::vector<std::pmr::vector<vk::AccelerationStructureGeometryKHR>> create_acceleration_structure_geometries(
        vk::Device const device,
        std::span<std::pmr::vector<Geometry_buffer_views> const> const geometry_buffer_views,
        World const& world,
        std::pmr::polymorphic_allocator<> const& output_allocator
    )
    {
        std::pmr::vector<std::pmr::vector<vk::AccelerationStructureGeometryKHR>> mesh_acceleration_structure_geometries{ output_allocator };

        for (std::size_t mesh_index = 0; mesh_index < world.meshes.size(); ++mesh_index)
        {
            Mesh const& mesh = world.meshes[mesh_index];

            std::pmr::vector<vk::AccelerationStructureGeometryKHR> acceleration_structure_geometries{ output_allocator };
            acceleration_structure_geometries.reserve(mesh.primitives.size());

            for (std::size_t primitive_index = 0; primitive_index < mesh.primitives.size(); ++primitive_index)
            {
                Primitive const& primitive = mesh.primitives[primitive_index];

                if (primitive.mode != Primitive::Mode::Triangles)
                {
                    std::cerr << std::format("Only triangle primitives are supported for acceleration structures. Skipping Mesh {} ({}) primitive {}.\n", mesh_index, mesh.name.value_or("<unamed>"), primitive_index);
                    continue; // TODO potential problem
                }

                Geometry_buffer_views const& geometry_buffer_view =
                    geometry_buffer_views[mesh_index][primitive_index];

                {
                    vk::DeviceOrHostAddressConstKHR const vertex_data_device_address
                    {
                        device.getBufferAddressKHR({.buffer = geometry_buffer_view.position.buffer}) + geometry_buffer_view.position.offset,
                    };

                    vk::DeviceOrHostAddressConstKHR const index_data_device_address
                    {
                        device.getBufferAddressKHR({.buffer = geometry_buffer_view.index.buffer}) + geometry_buffer_view.index.offset,
                    };

                    Accessor const& position_accessor = get_position_accessor(world, primitive);

                    vk::AccelerationStructureGeometryKHR const acceleration_structure_geometry
                    {
                        .geometryType = vk::GeometryTypeKHR::eTriangles,
                        .geometry =
                            vk::AccelerationStructureGeometryTrianglesDataKHR
                            {
                                .vertexFormat = vk::Format::eR32G32B32Sfloat,
                                .vertexData = vertex_data_device_address,
                                .vertexStride = sizeof(Maia::Scene::Vector3f),
                                .maxVertex = static_cast<std::uint32_t>(position_accessor.count),
                                .indexType = get_primitive_index_type(world, primitive),
                                .indexData = index_data_device_address,
                                .transformData = {},
                            },
                        .flags = vk::GeometryFlagBitsKHR::eOpaque,
                    };

                    acceleration_structure_geometries.push_back(acceleration_structure_geometry);
                }
            }

            mesh_acceleration_structure_geometries.push_back(std::move(acceleration_structure_geometries));
        }

        return mesh_acceleration_structure_geometries;
    }

    vk::AccelerationStructureBuildSizesInfoKHR create_acceleration_structure_build_sizes_info(
        vk::Device const device,
        std::span<vk::AccelerationStructureGeometryKHR const> const acceleration_structure_geometries,
        World const& world,
        Mesh const& mesh,
        std::pmr::polymorphic_allocator<> const& temporaries_allocator
    )
    {
        vk::AccelerationStructureBuildGeometryInfoKHR const acceleration_structure_build_geometry_info
        {
            .type = vk::AccelerationStructureTypeKHR::eBottomLevel,
            .flags = vk::BuildAccelerationStructureFlagBitsKHR::ePreferFastTrace,
            .geometryCount = static_cast<std::uint32_t>(acceleration_structure_geometries.size()),
            .pGeometries = acceleration_structure_geometries.data(),
        };

        std::pmr::vector<std::uint32_t> max_triangle_counts{ temporaries_allocator };
        max_triangle_counts.resize(mesh.primitives.size(), 0);
        for (std::size_t primitive_index = 0; primitive_index < mesh.primitives.size(); ++primitive_index)
        {
            Primitive const& primitive = mesh.primitives[primitive_index];
            Accessor const& indices_accessor = world.accessors[*primitive.indices_index];
            assert(indices_accessor.count <= std::numeric_limits<std::uint32_t>::max());
            max_triangle_counts[primitive_index] = static_cast<std::uint32_t>(indices_accessor.count);
        }

        vk::AccelerationStructureBuildSizesInfoKHR const acceleration_structure_build_sizes_info =
            device.getAccelerationStructureBuildSizesKHR(
                vk::AccelerationStructureBuildTypeKHR::eDevice,
                acceleration_structure_build_geometry_info,
                max_triangle_counts
            );

        return acceleration_structure_build_sizes_info;
    }

    std::pmr::vector<vk::AccelerationStructureBuildSizesInfoKHR> create_acceleration_structure_build_sizes_infos(
        vk::Device const device,
        std::span<std::pmr::vector<vk::AccelerationStructureGeometryKHR> const> const acceleration_structure_geometries_per_mesh,
        World const& world,
        std::pmr::polymorphic_allocator<> const& output_allocator,
        std::pmr::polymorphic_allocator<> const& temporaries_allocator
    )
    {
        auto const to_acceleration_structure_build_size = [&](std::size_t const mesh_index) -> vk::AccelerationStructureBuildSizesInfoKHR
        {
            return create_acceleration_structure_build_sizes_info(
                device,
                acceleration_structure_geometries_per_mesh[mesh_index],
                world,
                world.meshes[mesh_index],
                temporaries_allocator
            );
        };

        return
            std::views::iota(std::size_t{ 0 }, world.meshes.size()) |
            std::views::transform(to_acceleration_structure_build_size) |
            to_vector<vk::AccelerationStructureBuildSizesInfoKHR>(output_allocator);
    }

    Acceleration_structure create_acceleration_structure(
        vk::Device const device,
        Maia::Renderer::Vulkan::Buffer_resources& acceleration_structure_storage_buffer_resources,
        vk::AccelerationStructureBuildSizesInfoKHR const& acceleration_structure_build_sizes_info,
        vk::AccelerationStructureTypeKHR const type,
        vk::AllocationCallbacks const* const allocation_callbacks
    )
    {
        Maia::Renderer::Vulkan::Buffer_view const acceleration_structure_buffer_view =
            acceleration_structure_storage_buffer_resources.allocate_buffer(
                acceleration_structure_build_sizes_info.accelerationStructureSize,
                vk::MemoryPropertyFlagBits::eDeviceLocal
            );

        vk::AccelerationStructureCreateInfoKHR const acceleration_structure_create_info
        {
            .buffer = acceleration_structure_buffer_view.buffer,
            .offset = acceleration_structure_buffer_view.offset,
            .size = acceleration_structure_buffer_view.size,
            .type = type,
        };

        vk::AccelerationStructureKHR const acceleration_structure = device.createAccelerationStructureKHR(
            acceleration_structure_create_info,
            allocation_callbacks
        );

        vk::DeviceAddress const device_address = device.getAccelerationStructureAddressKHR({ .accelerationStructure = acceleration_structure });

        return Acceleration_structure
        {
            .handle = acceleration_structure,
            .device_address = device_address,
            .buffer_view = acceleration_structure_buffer_view,
        };
    }

    std::pmr::vector<Acceleration_structure> create_bottom_level_acceleration_structures(
        vk::Device const device,
        Maia::Renderer::Vulkan::Buffer_resources& acceleration_structure_storage_buffer_resources,
        std::span<std::pmr::vector<vk::AccelerationStructureGeometryKHR> const> const mesh_acceleration_structure_geometries,
        std::span<vk::AccelerationStructureBuildSizesInfoKHR const> const acceleration_structure_build_sizes_infos,
        World const& world,
        std::span<std::pmr::vector<Geometry_buffer_views> const> const geometry_buffer_views,
        vk::AllocationCallbacks const* const allocation_callbacks,
        std::pmr::polymorphic_allocator<> const& output_allocator
    )
    {
        auto const to_bottom_level_acceleration_structure = [&](std::size_t const mesh_index) -> Acceleration_structure
        {
            return create_acceleration_structure(
                device,
                acceleration_structure_storage_buffer_resources,
                acceleration_structure_build_sizes_infos[mesh_index],
                vk::AccelerationStructureTypeKHR::eBottomLevel,
                allocation_callbacks
            );
        };

        return
            std::views::iota(std::size_t{ 0 }, world.meshes.size()) |
            std::views::transform(to_bottom_level_acceleration_structure) |
            to_vector<Acceleration_structure>(output_allocator);
    }

    void build_bottom_level_acceleration_structures(
        vk::Device const device,
        vk::CommandBuffer const command_buffer,
        Maia::Renderer::Vulkan::Buffer_resources& scratch_buffer_resources,
        std::span<Acceleration_structure const> const acceleration_structures,
        std::span<std::pmr::vector<vk::AccelerationStructureGeometryKHR> const> const acceleration_structure_geometries,
        std::span<vk::AccelerationStructureBuildSizesInfoKHR const> const acceleration_structure_build_sizes_infos,
        World const& world,
        std::pmr::polymorphic_allocator<> const& temporaries_allocator
    )
    {
        assert(world.meshes.size() == acceleration_structures.size());
        assert(acceleration_structures.size() == acceleration_structure_build_sizes_infos.size());

        for (std::size_t mesh_index = 0; mesh_index < world.meshes.size(); ++mesh_index)
        {
            Mesh const& mesh = world.meshes[mesh_index];
            Acceleration_structure const& acceleration_structure = acceleration_structures[mesh_index];
            vk::AccelerationStructureBuildSizesInfoKHR const& acceleration_structure_build_sizes_info =
                acceleration_structure_build_sizes_infos[mesh_index];

            Maia::Renderer::Vulkan::Buffer_view const scratch_buffer_view =
                scratch_buffer_resources.allocate_buffer(
                    acceleration_structure_build_sizes_info.buildScratchSize,
                    vk::MemoryPropertyFlagBits::eDeviceLocal
                );

            vk::DeviceAddress const scratch_buffer_device_address = device.getBufferAddressKHR(
                { .buffer = scratch_buffer_view.buffer }
            ) + scratch_buffer_view.offset;

            vk::AccelerationStructureBuildGeometryInfoKHR const acceleration_build_geometry_info
            {
                .type = vk::AccelerationStructureTypeKHR::eBottomLevel,
                .flags = vk::BuildAccelerationStructureFlagBitsKHR::ePreferFastTrace,
                .mode = vk::BuildAccelerationStructureModeKHR::eBuild,
                .dstAccelerationStructure = acceleration_structure.handle,
                .geometryCount = static_cast<std::uint32_t>(acceleration_structure_geometries[mesh_index].size()),
                .pGeometries = acceleration_structure_geometries[mesh_index].data(),
                .scratchData = scratch_buffer_device_address
            };

            std::pmr::vector<vk::AccelerationStructureBuildRangeInfoKHR> acceleration_structure_build_range_info{ temporaries_allocator };
            acceleration_structure_build_range_info.resize(mesh.primitives.size());

            for (std::size_t primitive_index = 0; primitive_index < mesh.primitives.size(); ++primitive_index)
            {
                Primitive const& primitive = mesh.primitives[primitive_index];
                Accessor const& indices_accessor = world.accessors[*primitive.indices_index];

                acceleration_structure_build_range_info[primitive_index] = vk::AccelerationStructureBuildRangeInfoKHR
                {
                    .primitiveCount = static_cast<std::uint32_t>(indices_accessor.count),
                    .primitiveOffset = 0,
                    .firstVertex = 0,
                    .transformOffset = 0,
                };
            }

            std::array<vk::AccelerationStructureBuildRangeInfoKHR*, 1> const acceleration_build_structure_range_infos =
            {
                acceleration_structure_build_range_info.data()
            };

            command_buffer.buildAccelerationStructuresKHR(
                1,
                &acceleration_build_geometry_info,
                acceleration_build_structure_range_infos.data()
            );
        }
    }

    std::pmr::vector<Acceleration_structure> create_bottom_level_acceleration_structures(
        vk::PhysicalDeviceType const physical_device_type,
        vk::Device const device,
        vk::Queue const queue,
        vk::CommandPool const command_pool,
        Maia::Renderer::Vulkan::Buffer_resources& acceleration_structure_storage_buffer_resources,
        Maia::Renderer::Vulkan::Buffer_resources& geometry_buffer_resources,
        Maia::Renderer::Vulkan::Buffer_resources& upload_buffer_resources,
        Maia::Renderer::Vulkan::Buffer_resources& scratch_buffer_resources,
        World const& world,
        std::span<std::pmr::vector<std::byte> const> const buffers_data,
        vk::AllocationCallbacks const* const allocation_callbacks,
        std::pmr::polymorphic_allocator<> const& output_allocator,
        std::pmr::polymorphic_allocator<> const& temporaries_allocator
    )
    {
        std::pmr::vector<std::pmr::vector<Geometry_buffer_views>> const geometry_buffer_views =
            create_geometry_buffers(
                geometry_buffer_resources,
                world,
                output_allocator
            );

        upload_geometry_data(
            physical_device_type,
            device,
            queue,
            command_pool,
            upload_buffer_resources,
            world,
            buffers_data,
            geometry_buffer_views,
            allocation_callbacks,
            temporaries_allocator
        );

        std::pmr::vector<std::pmr::vector<vk::AccelerationStructureGeometryKHR>> const acceleration_structure_geometries =
            create_acceleration_structure_geometries(
                device,
                geometry_buffer_views,
                world,
                temporaries_allocator
            );

        std::pmr::vector<vk::AccelerationStructureBuildSizesInfoKHR> const acceleration_structure_build_sizes_infos =
            create_acceleration_structure_build_sizes_infos(
                device,
                acceleration_structure_geometries,
                world,
                temporaries_allocator,
                temporaries_allocator
            );

        std::pmr::vector<Acceleration_structure> const acceleration_structures =
            create_bottom_level_acceleration_structures(
                device,
                acceleration_structure_storage_buffer_resources,
                acceleration_structure_geometries,
                acceleration_structure_build_sizes_infos,
                world,
                geometry_buffer_views,
                allocation_callbacks,
                output_allocator
            );

        vk::CommandBuffer const command_buffer = create_one_time_submit_command_buffer(device, command_pool);

        build_bottom_level_acceleration_structures(
            device,
            command_buffer,
            scratch_buffer_resources,
            acceleration_structures,
            acceleration_structure_geometries,
            acceleration_structure_build_sizes_infos,
            world,
            temporaries_allocator
        );

        submit_and_wait(device, queue, command_pool, command_buffer, allocation_callbacks);

        return acceleration_structures;
    }

    template<typename Data_type>
    void upload_data(
        vk::PhysicalDeviceType const physical_device_type,
        vk::Device const device,
        vk::Queue const queue,
        vk::CommandPool const command_pool,
        Maia::Renderer::Vulkan::Buffer_resources& upload_buffer_resources,
        Maia::Renderer::Vulkan::Buffer_view const buffer_view,
        std::span<Data_type const> const buffer_data_to_upload,
        vk::AllocationCallbacks const* const allocation_callbacks
    )
    {
        if (physical_device_type == vk::PhysicalDeviceType::eIntegratedGpu)
        {
            map_memory_and_copy_data(
                device,
                buffer_view,
                buffer_data_to_upload
            );
        }
        else
        {
            Maia::Renderer::Vulkan::Upload_buffer const upload_buffer
            {
                device,
                upload_buffer_resources,
                buffer_data_to_upload.size_bytes()
            };

            upload_data_through_upload_buffer<Data_type>(
                device,
                queue,
                command_pool,
                buffer_view,
                buffer_data_to_upload,
                upload_buffer,
                allocation_callbacks
                );
        }
    }

    kln::translator to_translation(Vector3f const& vector) noexcept
    {
        return kln::translator{ 1.0f, vector.x, vector.y, vector.z };
    }

    kln::rotor to_rotor(Quaternionf const& quaternion) noexcept
    {
        // TODO assert that quarternion is normalized

        std::array<float, 4> quaternion_coefficients{ quaternion.w, quaternion.x, quaternion.y, quaternion.z };

        kln::rotor rotor{};
        rotor.load_normalized(quaternion_coefficients.data());
        return rotor;
    }

    kln::motor calculate_motor(Node const& node) noexcept
    {
        kln::rotor const rotation = to_rotor(node.rotation);
        kln::translator const translation = to_translation(node.translation);
        return translation * rotation;
    }

    // TODO from node transform matrix to math matrix
    // TODO from math matrix to vk::TransformMatrixKHR
    kln::motor calculate_world_motor(kln::motor const& parent_world_motor, Node const& node) noexcept
    {
        kln::motor const local_motor = calculate_motor(node);
        return local_motor * parent_world_motor;
    }

    vk::TransformMatrixKHR to_transform_matrix(kln::motor const& motor)
    {
        kln::motor const normalized_motor = motor.normalized();
        kln::mat3x4 const column_major_transform = normalized_motor.as_mat3x4();

        // Column major 3x4
        // 0 3 6 9
        // 1 4 7 10
        // 2 5 8 11

        // Row major 3x4
        // 0 1 2 3
        // 4 5 6 7
        // 8 9 10 11

        vk::TransformMatrixKHR row_major_transform = {};

        row_major_transform.matrix[0][0] = column_major_transform.data[0 * 3 + 0];
        row_major_transform.matrix[0][1] = column_major_transform.data[1 * 3 + 0];
        row_major_transform.matrix[0][2] = column_major_transform.data[2 * 3 + 0];
        row_major_transform.matrix[0][3] = column_major_transform.data[3 * 3 + 0];

        row_major_transform.matrix[1][0] = column_major_transform.data[0 * 3 + 1];
        row_major_transform.matrix[1][1] = column_major_transform.data[1 * 3 + 1];
        row_major_transform.matrix[1][2] = column_major_transform.data[2 * 3 + 1];
        row_major_transform.matrix[1][3] = column_major_transform.data[3 * 3 + 1];

        row_major_transform.matrix[2][0] = column_major_transform.data[0 * 3 + 2];
        row_major_transform.matrix[2][1] = column_major_transform.data[1 * 3 + 2];
        row_major_transform.matrix[2][2] = column_major_transform.data[2 * 3 + 2];
        row_major_transform.matrix[2][3] = column_major_transform.data[3 * 3 + 2];

        return row_major_transform;
    }

    void calculate_child_nodes_world_matrices(Maia::Scene::World const& world, std::span<vk::TransformMatrixKHR> const world_matrices, Maia::Scene::Node const& parent_node, kln::motor const& parent_world_motor)
    {
        auto const calculate_child_nodes_world_matrices_aux = [&world, &world_matrices, &parent_world_motor](std::size_t const child_node_index) -> void
        {
            Maia::Scene::Node const& child_node = world.nodes[child_node_index];

            kln::motor const child_node_world_matrix = calculate_world_motor(parent_world_motor, child_node);
            world_matrices[child_node_index] = to_transform_matrix(child_node_world_matrix);

            calculate_child_nodes_world_matrices(world, world_matrices, child_node, child_node_world_matrix);
        };

        std::for_each(std::execution::par_unseq, parent_node.child_indices.begin(), parent_node.child_indices.end(), calculate_child_nodes_world_matrices_aux);
    }

    std::pmr::vector<vk::TransformMatrixKHR> create_instances_transforms(
        Maia::Scene::World const& world,
        Maia::Scene::Scene const& scene,
        std::pmr::polymorphic_allocator<> const& output_allocator
    )
    {
        if (scene.nodes)
        {
            std::pmr::vector<vk::TransformMatrixKHR> world_matrices{ output_allocator };
            world_matrices.resize(world.nodes.size());

            auto const calculate_world_matrices = [&world, &world_matrices](std::size_t const root_node_index) -> void
            {
                Maia::Scene::Node const& root_node = world.nodes[root_node_index];
                kln::motor const root_node_world_motor = calculate_motor(root_node);
                world_matrices[root_node_index] = to_transform_matrix(root_node_world_motor);

                calculate_child_nodes_world_matrices(world, world_matrices, root_node, root_node_world_motor);
            };

            std::for_each(std::execution::par_unseq, scene.nodes->begin(), scene.nodes->end(), calculate_world_matrices);

            return world_matrices;
        }
        else
        {
            return std::pmr::vector<vk::TransformMatrixKHR>{ output_allocator };
        }
    }

    Instance_buffer_view create_instances_buffer(
        Maia::Renderer::Vulkan::Buffer_resources& instances_buffer_resources,
        std::size_t const instance_count
    )
    {
        Maia::Renderer::Vulkan::Buffer_view const instances_buffer_view =
            instances_buffer_resources.allocate_buffer(
                instance_count * sizeof(vk::AccelerationStructureInstanceKHR),
                vk::MemoryPropertyFlagBits::eDeviceLocal
            );

        return instances_buffer_view;
    }

    template<typename Value_t, typename Execution_policy_t, typename Begin_t, typename End_t, typename Function_t>
    std::pmr::vector<Value_t> transform_and_output_to_vector(Execution_policy_t&& policy, Begin_t const begin, End_t const end, Function_t&& function, std::pmr::polymorphic_allocator<> const& allocator)
    {
        std::pmr::vector<Value_t> output{ allocator };
        output.resize(std::distance(begin, end));

        std::transform(policy, begin, end, output.begin(), function);

        return output;
    }

    template<typename Value_t, typename Execution_policy_t, typename Range_t, typename Function_t>
    std::pmr::vector<Value_t> transform_and_output_to_vector(Execution_policy_t&& policy, Range_t range, Function_t&& function, std::pmr::polymorphic_allocator<> const& allocator)
    {
        std::pmr::vector<Value_t> output{ allocator };
        output.resize(std::ranges::distance(range));

        std::transform(policy, std::ranges::begin(range), std::ranges::end(range), output.begin(), function);

        return output;
    }

    vk::AccelerationStructureGeometryKHR create_acceleration_structure_instance_geometry(
        vk::Device const device,
        Maia::Renderer::Vulkan::Buffer_view const& instance_buffer_view
    )
    {
        vk::DeviceOrHostAddressConstKHR const instance_data_device_address
        {
            device.getBufferAddressKHR({.buffer = instance_buffer_view.buffer}) + instance_buffer_view.offset,
        };

        vk::AccelerationStructureGeometryKHR const acceleraiton_structure_geometry
        {
            .geometryType = vk::GeometryTypeKHR::eInstances,
            .geometry = vk::AccelerationStructureGeometryInstancesDataKHR
            {
                .arrayOfPointers = false,
                .data = instance_data_device_address,
            },
            .flags = vk::GeometryFlagBitsKHR::eOpaque,
        };

        return acceleraiton_structure_geometry;
    }

    vk::AccelerationStructureBuildSizesInfoKHR create_acceleration_structure_build_sizes_info(
        vk::Device const device,
        vk::AccelerationStructureGeometryKHR const acceleration_structure_geometry
    )
    {
        vk::AccelerationStructureBuildGeometryInfoKHR const acceleration_structure_build_geometry_info
        {
            .type = vk::AccelerationStructureTypeKHR::eTopLevel,
            .flags = vk::BuildAccelerationStructureFlagBitsKHR::ePreferFastTrace,
            .geometryCount = 1,
            .pGeometries = &acceleration_structure_geometry,
        };

        std::uint32_t const primitive_count = 1;

        vk::AccelerationStructureBuildSizesInfoKHR const acceleration_structure_build_sizes_info =
            device.getAccelerationStructureBuildSizesKHR(
                vk::AccelerationStructureBuildTypeKHR::eDevice,
                acceleration_structure_build_geometry_info,
                primitive_count
            );

        return acceleration_structure_build_sizes_info;
    }

    void build_top_level_acceleration_structure(
        vk::Device const device,
        vk::CommandBuffer const command_buffer,
        Maia::Renderer::Vulkan::Buffer_resources& scratch_buffer_resources,
        vk::AccelerationStructureKHR const& acceleration_structure,
        vk::AccelerationStructureGeometryKHR const& acceleration_structure_geometry,
        vk::AccelerationStructureBuildSizesInfoKHR const& acceleration_structure_build_sizes_info
    )
    {
        Maia::Renderer::Vulkan::Buffer_view const scratch_buffer_view =
            scratch_buffer_resources.allocate_buffer(
                acceleration_structure_build_sizes_info.buildScratchSize,
                vk::MemoryPropertyFlagBits::eDeviceLocal
            );

        vk::DeviceAddress const scratch_buffer_device_address = device.getBufferAddressKHR(
            { .buffer = scratch_buffer_view.buffer }
        ) + scratch_buffer_view.offset;

        vk::AccelerationStructureBuildGeometryInfoKHR const acceleration_build_geometry_info
        {
            .type = vk::AccelerationStructureTypeKHR::eTopLevel,
            .flags = vk::BuildAccelerationStructureFlagBitsKHR::ePreferFastTrace,
            .mode = vk::BuildAccelerationStructureModeKHR::eBuild,
            .dstAccelerationStructure = acceleration_structure,
            .geometryCount = 1,
            .pGeometries = &acceleration_structure_geometry,
            .scratchData = scratch_buffer_device_address,
        };

        vk::AccelerationStructureBuildRangeInfoKHR const acceleration_structure_build_range_info
        {
            .primitiveCount = 1,
            .primitiveOffset = 0,
            .firstVertex = 0,
            .transformOffset = 0,
        };
        std::array<vk::AccelerationStructureBuildRangeInfoKHR const*, 1> const acceleration_build_structure_range_infos = { &acceleration_structure_build_range_info };

        command_buffer.buildAccelerationStructuresKHR(
            1,
            &acceleration_build_geometry_info,
            acceleration_build_structure_range_infos.data()
        );
    }

    std::pmr::vector<Acceleration_structure> create_top_level_acceleration_structures(
        vk::PhysicalDeviceType const physical_device_type,
        vk::Device const device,
        vk::Queue const queue,
        vk::CommandPool const command_pool,
        std::span<Acceleration_structure const> const bottom_level_acceleration_structures,
        Maia::Renderer::Vulkan::Buffer_resources& acceleration_structure_storage_buffer_resources,
        Maia::Renderer::Vulkan::Buffer_resources& instance_buffer_resources,
        Maia::Renderer::Vulkan::Buffer_resources& upload_buffer_resources,
        Maia::Renderer::Vulkan::Buffer_resources& scratch_buffer_resources,
        Maia::Scene::World const& world,
        Maia::Scene::Scene const& scene,
        vk::AllocationCallbacks const* const allocation_callbacks,
        std::pmr::polymorphic_allocator<> const& output_allocator,
        std::pmr::polymorphic_allocator<> const& temporaries_allocator
    )
    {
        if (!scene.nodes.has_value())
        {
            return {};
        }

        std::pmr::vector<vk::TransformMatrixKHR> const instances_transforms = create_instances_transforms(world, scene, temporaries_allocator);

        auto const create_acceleration_structure_instance = [&](std::size_t const node_index) -> vk::AccelerationStructureInstanceKHR
        {
            assert(world.nodes[node_index].mesh_index.has_value());

            std::size_t const mesh_index = *world.nodes[node_index].mesh_index;

            return vk::AccelerationStructureInstanceKHR
            {
                .transform = instances_transforms[node_index],
                .instanceCustomIndex = static_cast<std::uint32_t>(node_index),
                .mask = 0xFF,
                .instanceShaderBindingTableRecordOffset = 0,
                .flags = VK_GEOMETRY_INSTANCE_TRIANGLE_FACING_CULL_DISABLE_BIT_KHR, //vk::GeometryInstanceFlagBitsKHR::eTriangleFacingCullDisable,
                .accelerationStructureReference = bottom_level_acceleration_structures[mesh_index].device_address,
            };
        };

        auto const mesh_nodes = *scene.nodes | std::views::filter([&world](std::size_t const node_index) -> bool { return world.nodes[node_index].mesh_index.has_value(); });

        std::pmr::vector<vk::AccelerationStructureInstanceKHR> const acceleration_structure_instances =
            transform_and_output_to_vector<vk::AccelerationStructureInstanceKHR>(
                std::execution::par_unseq,
                mesh_nodes,
                create_acceleration_structure_instance,
                temporaries_allocator
                );

        auto const create_instance_buffer_view = [&](vk::AccelerationStructureInstanceKHR const acceleration_structure_instance) -> Maia::Renderer::Vulkan::Buffer_view
        {
            Maia::Renderer::Vulkan::Buffer_view const instance_buffer_view =
                instance_buffer_resources.allocate_buffer(
                    sizeof(VkAccelerationStructureInstanceKHR),
                    vk::MemoryPropertyFlagBits::eDeviceLocal
                );

            upload_data<vk::AccelerationStructureInstanceKHR>(
                physical_device_type,
                device,
                queue,
                command_pool,
                upload_buffer_resources,
                instance_buffer_view,
                { &acceleration_structure_instance , 1 },
                allocation_callbacks
                );

            return instance_buffer_view;
        };

        std::pmr::vector<Maia::Renderer::Vulkan::Buffer_view> const instance_buffer_views =
            transform_and_output_to_vector<Maia::Renderer::Vulkan::Buffer_view>(
                std::execution::seq,
                acceleration_structure_instances.begin(),
                acceleration_structure_instances.end(),
                create_instance_buffer_view,
                temporaries_allocator
                );

        std::pmr::vector<vk::AccelerationStructureGeometryKHR> const acceleration_structure_geometries =
            transform_and_output_to_vector<vk::AccelerationStructureGeometryKHR>(
                std::execution::seq,
                instance_buffer_views.begin(),
                instance_buffer_views.end(),
                [device](Maia::Renderer::Vulkan::Buffer_view const& instance_buffer_view) { return create_acceleration_structure_instance_geometry(device, instance_buffer_view); },
                temporaries_allocator
                );

        std::pmr::vector<vk::AccelerationStructureBuildSizesInfoKHR> const acceleration_structure_build_sizes_infos =
            transform_and_output_to_vector<vk::AccelerationStructureBuildSizesInfoKHR>(
                std::execution::seq,
                acceleration_structure_geometries.begin(),
                acceleration_structure_geometries.end(),
                [device](vk::AccelerationStructureGeometryKHR const& acceleration_structure_geometry) { return create_acceleration_structure_build_sizes_info(device, acceleration_structure_geometry); },
                temporaries_allocator
                );

        std::pmr::vector<Acceleration_structure> const acceleration_structures =
            transform_and_output_to_vector<Acceleration_structure>(
                std::execution::seq,
                acceleration_structure_build_sizes_infos.begin(),
                acceleration_structure_build_sizes_infos.end(),
                [&](vk::AccelerationStructureBuildSizesInfoKHR const& build_size_info) { return create_acceleration_structure(device, acceleration_structure_storage_buffer_resources, build_size_info, vk::AccelerationStructureTypeKHR::eTopLevel, allocation_callbacks); },
                temporaries_allocator
                );

        vk::CommandBuffer const command_buffer = create_one_time_submit_command_buffer(device, command_pool);

        auto const build_top_level_acceleration_structure_lambda = [&](std::size_t const acceleration_structure_index) -> void
        {
            build_top_level_acceleration_structure(
                device,
                command_buffer,
                scratch_buffer_resources,
                acceleration_structures[acceleration_structure_index].handle,
                acceleration_structure_geometries[acceleration_structure_index],
                acceleration_structure_build_sizes_infos[acceleration_structure_index]
            );
        };

        auto mesh_nodes_copy = mesh_nodes;
        std::ranges::for_each(mesh_nodes_copy, build_top_level_acceleration_structure_lambda);

        submit_and_wait(device, queue, command_pool, command_buffer, allocation_callbacks);

        return acceleration_structures;
    }
}
