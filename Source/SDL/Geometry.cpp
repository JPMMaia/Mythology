module;

#include <vulkan/vulkan.hpp>

#include <algorithm>
#include <array>
#include <cassert>
#include <filesystem>
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
    // Create Bottom Level geometry buffers
    // Input [Mesh]
    // Output [[{Position_buffer_view, Index_buffer_view}]]
    // Mesh -> [Primitive]
    // Primitive -> Position Accessor -> Position vertex buffer
    // Primitive -> Index Accessor -> Index buffer

    // Create Bottom Level acceleration structures
    // Input [Mesh], [[{Position_buffer_view, Index_buffer_view}]]
    // Output [Bottom Level Acceleration Structure]

    // Top level
    // Input [Node that contains mesh], [Mesh], [Bottom level acceleration structure]
    // Output [{Transform_buffer_view, Mesh*}]
    // Node that contains mesh -> Transform buffer
    // Node that contains mesh -> Mesh*

    struct Geometry_buffer_views
    {
        Maia::Renderer::Vulkan::Buffer_view position = {};
        Maia::Renderer::Vulkan::Buffer_view index = {};
    };

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
                .index = 1
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
        assert(data_to_upload.size() > upload_buffer.buffer_view().size);

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

    void create_bottom_level_acceleration_structures(
        vk::PhysicalDeviceType const physical_device_type,
        vk::Device const device,
        vk::Queue const queue,
        vk::CommandPool const command_pool,
        Maia::Renderer::Vulkan::Buffer_resources& geometry_buffer_resources,
        Maia::Renderer::Vulkan::Buffer_resources& upload_buffer_resources,
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

        // TODO Create bottom level acceleration structures
        // TODO Build bottom level accelearation structures
    }

    // Create instance buffers
    // Upload data
    // Create top level acceleration structures
    // Build top level acceleration structures
    /*
        Acceleration_structure create_bottom_level_acceleration_structure(
            vk::Device const device,
            vk::CommandBuffer const command_buffer,
            Maia::Renderer::Vulkan::Buffer_resources& acceleration_structure_build_input_buffer_resources, // vk::BufferUsageFlagBits::eAccelerationStructureBuildInputReadOnlyKHR | vk::BufferUsageFlagBits::eShaderDeviceAddressKHR
            Maia::Renderer::Vulkan::Buffer_resources& acceleration_structure_storage_buffer_resources, // vk::BufferUsageFlagBits::eAccelerationStructureStorageKHR
            Maia::Renderer::Vulkan::Buffer_resources& scratch_buffer_resources, // vk::BufferUsage::eStorageBuffer | vk::BufferUsage::eShaderDeviceAddressKHR
            vk::AllocationCallbacks const* const allocation_callbacks
        )
        {
            // Setup vertices and indices for a single triangle
            struct Vertex
            {
                float pos[3];
            };

            std::vector<Vertex> const vertices =
            {
                {{1.0f, 1.0f, 0.0f}},
                {{-1.0f, 1.0f, 0.0f}},
                {{0.0f, -1.0f, 0.0f}}
            };
            std::vector<std::uint32_t> const indices = { 0, 1, 2 };

            std::size_t const vertex_buffer_size = vertices.size() * sizeof(Vertex);
            std::size_t const index_buffer_size = indices.size() * sizeof(std::uint32_t);
            std::size_t const transform_matrix_buffer_size = sizeof(vk::TransformMatrixKHR);

            // Create buffers for the bottom level geometry
            // For the sake of simplicity we won't stage the vertex data to the GPU memory

            // Note that the buffer usage flags for buffers consumed by the bottom level acceleration structure require special flags
            vk::BufferUsageFlags const buffer_usage_flags = vk::BufferUsageFlagBits::eAccelerationStructureBuildInputReadOnlyKHR | vk::BufferUsageFlagBits::eShaderDeviceAddressKHR;

            Maia::Renderer::Vulkan::Buffer_view const vertex_buffer_view =
                acceleration_structure_build_input_buffer_resources.allocate_buffer(vertex_buffer_size, vk::MemoryPropertyFlagBits::eDeviceLocal);
            vertex_buffer_view->update(vertices.data(), vertex_buffer_size); // TODO

            Maia::Renderer::Vulkan::Buffer_view const index_buffer_view =
                acceleration_structure_build_input_buffer_resources.allocate_buffer(index_buffer_size, vk::MemoryPropertyFlagBits::eDeviceLocal);
            index_buffer_view->update(indices.data(), index_buffer_size); // TODO

            vk::DeviceOrHostAddressConstKHR const vertex_data_device_address
            {
                .deviceAddress = device.getBufferDeviceAddressKHR({.buffer = vertex_buffer_view.buffer}) + vertex_buffer_view.offset,
            };

            vk::DeviceOrHostAddressConstKHR const index_data_device_address
            {
                .deviceAddress = device.getBufferDeviceAddressKHR({.buffer = index_buffer_view.buffer}) + index_buffer_view.offset,
            };

            // The bottom level acceleration structure contains one set of triangles as the input geometry
            vk::AccelerationStructureGeometryKHR const acceleration_structure_geometry
            {
                .geometryType = vk::GeometryTypeKHR::eTriangles,
                .flags = vk::GeometryFlagBitsKHR::eOpaque VK_GEOMETRY_OPAQUE_BIT_KHR,
                .geometry =
                {
                    .triangles =
                    {
                        .vertexFormat = vk::Format::eR32G32B32Sfloat,
                        .vertexData = vertex_data_device_address,
                        .maxVertex = static_cast<std::uint32_t>(vertices.size()),
                        .vertexStride = sizeof(Vertex),
                        .indexType = vk::IndexType::eUint32,
                        .indexData = index_data_device_address,
                        .transformData = {},
                    }
                }
            };

            // Get the size requirements for buffers involved in the acceleration structure build process
            vk::AccelerationStructureBuildGeometryInfoKHR const acceleration_structure_build_geometry_info
            {
                .type = vk::AccelerationStructureTypeKHR::eBottomLevel,
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

            // Create a buffer to hold the acceleration structure
            Maia::Renderer::Vulkan::Buffer_view const bottom_level_acceleration_structure_buffer_view =
                acceleration_structure_storage_buffer_resources.allocate_buffer(
                    acceleration_structure_build_sizes_info.accelerationStructureSize,
                    vk::MemoryPropertyFlagBits::eDeviceLocal
                );

            // Create the acceleration structure
            vk::AccelerationStructureCreateInfoKHR const acceleration_structure_create_info
            {
                .buffer = bottom_level_acceleration_structure_buffer_view.buffer,
                .offset = bottom_level_acceleration_structure_buffer_view.offset
                .size = bottom_level_acceleration_structure_buffer_view.size,
                .type = vk::AccelerationStructureTypeKHR::eBottomLevel,
            };

            vk::AccelerationStructureKHR const bottom_level_acceleration_structure = device.createAccelerationStructureKHR(
                acceleration_structure_create_info,
                allocation_callbacks
            );

            // The actual build process starts here

            // Create a scratch buffer as a temporary storage for the acceleration structure build
            Maia::Renderer::Vulkan::Buffer_view const scratch_buffer_view =
                scratch_buffer_resources.allocate_buffer(
                    acceleration_structure_build_sizes_info.buildScratchSize,
                    vk::MemoryPropertyFlagBits::eDeviceLocal
                );
            vk::DeviceAddress const scratch_buffer_device_address = device.getBufferDeviceAddressKHR({ .buffer = scratch_buffer_view.buffer }) + scratch_buffer_view.offset;

            vk::AccelerationStructureBuildGeometryInfoKHR const acceleration_build_geometry_info
            {
                .type = vk::AccelerationStructureTypeKHR::eBottomLevel,
                .flags = vk::BuildAccelerationStructureFlagBitsKHR::ePreferFastTrace,
                .mode = vk::BuildAccelerationStructureModeKHR::eBuild,
                .dstAccelerationStructure = bottom_level_acceleration_structure,
                .geometryCount = 1,
                .pGeometries = &acceleration_structure_geometry,
                .scratchData =
                {
                    .deviceAddress = scratch_buffer_device_address,
                },
            };

            vk::AccelerationStructureBuildRangeInfoKHR const acceleration_structure_build_range_info
            {
                .primitiveCount = 1,
                .primitiveOffset = 0,
                .firstVertex = 0,
                .transformOffset = 0,
            };

            std::array<vk::AccelerationStructureBuildRangeInfoKHR*, 1> const acceleration_build_structure_range_infos =
            {
                &acceleration_structure_build_range_info
            };

            // Build the acceleration structure on the device via a one-time command buffer submission
            // Some implementations may support acceleration structure building on the host (VkPhysicalDeviceAccelerationStructureFeaturesKHR->accelerationStructureHostCommands), but we prefer device builds

            command_buffer.buildAccelerationStructuresKHR(
                1,
                &acceleration_build_geometry_info,
                acceleration_build_structure_range_infos.data()
            );

            // Get the bottom acceleration structure's handle, which will be used during the top level acceleration build
            vk::DeviceAddress const device_address = device.getAccelerationStructureDeviceAddressKHR({ .accelerationStructure = bottom_level_acceleration_structure });

            return
            {
                .handle = bottom_level_acceleration_structure,
                .device_address = device_address,
                .buffer_view = bottom_level_acceleration_structure_buffer_view,
            };
        }

        Acceleration_structure create_top_level_acceleration_structure(
            vk::Device const device,
            vk::CommandBuffer const command_buffer,
            Acceleration_structure const bottom_level_acceleration_structure,
            Maia::Renderer::Vulkan::Buffer_resources& acceleration_structure_build_input_buffer_resources, // vk::BufferUsageFlagBits::eAccelerationStructureBuildInputReadOnlyKHR | vk::BufferUsageFlagBits::eShaderDeviceAddressKHR
            Maia::Renderer::Vulkan::Buffer_resources& acceleration_structure_storage_buffer_resources, // vk::BufferUsageFlagBits::eAccelerationStructureStorageKHR
            Maia::Renderer::Vulkan::Buffer_resources& scratch_buffer_resources, // vk::BufferUsage::eStorageBuffer | vk::BufferUsage::eShaderDeviceAddressKHR
            vk::AllocationCallbacks const* const allocation_callbacks
        )
        {
            vk::TransformMatrixKHR const transform_matrix =
            {
                1.0f, 0.0f, 0.0f, 0.0f,
                0.0f, 1.0f, 0.0f, 0.0f,
                0.0f, 0.0f, 1.0f, 0.0f
            };

            vk::AccelerationStructureInstanceKHR const acceleration_structure_instance
            {
                .transform = transform_matrix,
                .instanceCustomIndex = 0,
                .mask = 0xFF,
                .instanceShaderBindingTableRecordOffset = 0,
                .flags = vk::GeometryInstanceFlagBitsKHR::eTriangleFacingCullDisable,
                .accelerationStructureReference = bottom_level_acceleration_structure.device_address,
            };

            Maia::Renderer::Vulkan::Buffer_view const instances_buffer_view =
                acceleration_structure_build_input_buffer_resources.allocate_buffer(
                    sizeof(VkAccelerationStructureInstanceKHR),
                    vk::MemoryPropertyFlagBits::eDeviceLocal
                );
            instances_buffer_view->update(&acceleration_structure_instance, sizeof(VkAccelerationStructureInstanceKHR)); // TODO

            vk::DeviceOrHostAddressConstKHR const instance_data_device_address
            {
                .deviceAddress = device.getBufferDeviceAddressKHR({.buffer = instances_buffer_view.buffer}) + instances_buffer_view.offset,
            };

            // The top level acceleration structure contains (bottom level) instance as the input geometry
            vk::AccelerationStructureGeometryKHR const acceleration_structure_geometry
            {
                .geometryType = vk::GeometryTypeKHR::eInstances,
                .flags = vk::GeometryFlagBitsKHR::eOpaque,
                .geometry =
                {
                    .instances =
                    {
                        .arrayOfPointers = false,
                        .data = instance_data_device_address,
                    },
                },
            };

            // Get the size requirements for buffers involved in the acceleration structure build process
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
                    primitive_count,
                    acceleration_structure_build_sizes_info
                );

            // Create a buffer to hold the acceleration structure
            Maia::Renderer::Vulkan::Bufer_view const top_level_acceleration_structure_buffer_view =
                acceleration_structure_storage_buffer_resources.allocate_buffer(
                    acceleration_structure_build_sizes_info.accelerationStructureSize,
                    vk::MemoryPropertyFlagBits::eDeviceLocal
                );

            // Create the acceleration structure
            vk::AccelerationStructureCreateInfoKHR const acceleration_structure_create_info
            {
                .buffer = top_level_acceleration_structure_buffer_view.buffer,
                .offset = top_level_acceleration_structure_buffer_view.offset,
                .size = acceleration_structure_build_sizes_info.accelerationStructureSize,
                .type = vk::AccelerationStructureTypeKHR::eTopLevel,
            };
            vk::AccelerationStructureKHR const top_level_acceleration_structure = device.createAccelerationStructureKHR(acceleration_structure_create_info, allocation_callbacks);

            // The actual build process starts here

            // Create a scratch buffer as a temporary storage for the acceleration structure build
            Maia::Renderer::Vulkan::Buffer_view const scratch_buffer_view =
                scratch_buffer_resources.allocate_buffer(
                    acceleration_structure_build_sizes_info.buildScratchSize,
                    vk::MemoryPropertyFlagBits::eDeviceLocal
                );
            vk::DeviceAddress const scratch_buffer_device_address = device.getBufferDeviceAddressKHR({ .buffer = scratch_buffer_view.buffer }) + scratch_buffer_view.offset;

            vk::AccelerationStructureBuildGeometryInfoKHR const acceleration_build_geometry_info
            {
                .type = vk::AccelerationStructureTypeKHR::eTopLevel,
                .flags = vk::BuildAccelerationStructureFlagBitsKHR::ePreferFastTrace,
                .mode = vk::BuildAccelerationStructureModeKHR::eBuild,
                .dstAccelerationStructure = top_level_acceleration_structure,
                .geometryCount = 1,
                .pGeometries = &acceleration_structure_geometry,
                .scratchData =
                {
                    .deviceAddress = scratch_buffer_device_address,
                },
            };

            vk::AccelerationStructureBuildRangeInfoKHR const acceleration_structure_build_range_info
            {
                .primitiveCount = 1,
                .primitiveOffset = 0,
                .firstVertex = 0,
                .transformOffset = 0,
            };
            std::array<vk::AccelerationStructureBuildRangeInfoKHR*, 1> const acceleration_build_structure_range_infos = { &acceleration_structure_build_range_info };

            // Build the acceleration structure on the device via a one-time command buffer submission
            // Some implementations may support acceleration structure building on the host (VkPhysicalDeviceAccelerationStructureFeaturesKHR->accelerationStructureHostCommands), but we prefer device builds

            command_buffer.buildAccelerationStructuresKHR(
                1,
                &acceleration_build_geometry_info,
                acceleration_build_structure_range_infos.data()
            );

            // Get the top acceleration structure's handle, which will be used to setup it's descriptor
            vk::DeviceAddress const device_address = device.getAccelerationStructureDeviceAddressKHR({ .accelerationStructure = top_level_acceleration_structure });

            return
            {
                .handle = top_level_acceleration_structure,
                .device_address = device_address,
                .buffer_view = top_level_acceleration_structure_buffer_view,
            };
        }

        */
}
