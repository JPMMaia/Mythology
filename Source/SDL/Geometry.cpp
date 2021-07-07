module;

#include <vulkan/vulkan.hpp>

#include <memory_resource>
#include <vector>

module mythology.geometry;

namespace Mythology
{
    struct Scratch_buffer
    {
        Scratch_buffer(
            vk::Device const device,
            vk::DeviceSize const size,
            vk::AllocationCallbacks const* const allocation_callbacks
        );
        Scratch_buffer(Scratch_buffer const&) = delete;
        Scratch_buffer(Scratch_buffer&& other) noexcept;
        ~Scratch_buffer() noexcept;

        Scratch_buffer& operator=(Scratch_buffer const&) = delete;
        Scratch_buffer& operator=(Scratch_buffer&& other) noexcept;

        vk::Device device = {};
        vk::AllocationCallbacks const* allocation_callbacks = nullptr;
        std::uint64_t device_address = 0;
        vk::Buffer handle = {};
        vk::DeviceMemory memory = {};
    };

    Scratch_buffer::Scratch_buffer(
        vk::Device const device,
        vk::DeviceSize const size,
        vk::AllocationCallbacks const* const allocation_callbacks
    )
    {
        vk::BufferCreateInfo const buffer_create_info
        {
            .size = size,
            .usage = vk::BufferUsage::eStorageBuffer | vk::BufferUsage::eShaderDeviceAddressKHR,
        };

        vk::Buffer const buffer = device.createBuffer(buffer_create_info, allocation_callbacks);

        vk::MemoryRequirements const memory_requirements = device.getBufferMemoryRequirements(buffer);

        vk::MemoryAllocateFlagsInfo const memory_allocate_flags_info
        {
            .flags = vk::MemoryAllocateFlagBits::eDeviceAddress,
        };

        vk::MemoryAllocateInfo const memory_allocate_info
        {
            .pNext = &memory_allocate_flags_info,
            .allocationSize = memory_requirements.size,
            .memoryTypeIndex = device->get_memory_type(memory_requirements.memoryTypeBits, vk::MemoryPropertyFlagBits::eDeviceLocal),
        }

        vk::DeviceMemory const memory = device.allocateMemory(memory_allocate_info, allocation_callbacks);

        vk::DeviceAddress const offet = 0;
        device.bindBufferMemory(buffer, memory, offet);

        std::uint64_t const device_address = device.getBufferDeviceAddressKHR({.buffer = buffer});

        this->device = device;
        this->device_address = device_address;
        this->handle = buffer;
        this->memory = memory;
    }

    Scratch_buffer::Scratch_buffer(Scratch_buffer&& other) noexcept :
        device{std::exchange(other.device, vk::Device{})},
        device_address{std::exchange(other.device_address, vk::DeviceAddress{})},
        handle{std::exchange(other.handle, vk::Buffer{})},
        memory{std::exchange(other.memory, vk::DeviceMemory{})},
    {
    }

    Scratch_buffer::~Scratch_buffer() noexcept
    {
        if (this->memory != vk::DeviceMemory{})
        {
            this->device.free(this->memory, this->allocation_callbacks);
        }
        
        if (this->handle != vk::Buffer{})
        {
            this->device.destroy(this->handle, this->allocation_callbacks);
        }
    }

    Scratch_buffer& Scratch_buffer::operator=(Scratch_buffer&& other) noexcept
    {
        std::swap(this->device, other.device);
        std::swap(this->device_address, other.device_address);
        std::swap(this->handle, other.handle);
        std::swap(this->memory, other.memory);

        return *this;
    }

    struct Acceleration_structure
    {
        vk::AccelerationStructureKHR handle = {};
        std::uint64_t device_address = 0;
        vk::Buffer buffer = {};
    };

    Acceleration_structure create_bottom_level_acceleration_structure(
        vk::Device const device,
        vk::CommandBuffer const command_buffer,
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
        std::vector<std::uint32_t> const indices = {0, 1, 2};

        std::size_t vertex_buffer_size = vertices.size() * sizeof(Vertex);
        std::size_t index_buffer_size  = indices.size() * sizeof(std::uint32_t);

        // Create buffers for the bottom level geometry
        // For the sake of simplicity we won't stage the vertex data to the GPU memory

        // Note that the buffer usage flags for buffers consumed by the bottom level acceleration structure require special flags
        vk::BufferUsageFlags const buffer_usage_flags = vk::BufferUsageFlagBits::eAccelerationStructureBuildInputReadOnlyKHR | vk::BufferUsageFlagBits::eShaderDeviceAddressKHR;

        vk::Buffer const vertex_buffer = std::make_unique<vkb::core::Buffer>(get_device(), vertex_buffer_size, buffer_usage_flags, VMA_MEMORY_USAGE_CPU_TO_GPU);
        vertex_buffer->update(vertices.data(), vertex_buffer_size);

        vk::Buffer const index_buffer = std::make_unique<vkb::core::Buffer>(get_device(), index_buffer_size, buffer_usage_flags, VMA_MEMORY_USAGE_CPU_TO_GPU);
        index_buffer->update(indices.data(), index_buffer_size);

        // Setup a single transformation matrix that can be used to transform the whole geometry for a single bottom level acceleration structure
        vk::TransformMatrixKHR transform_matrix = {
            1.0f, 0.0f, 0.0f, 0.0f,
            0.0f, 1.0f, 0.0f, 0.0f,
            0.0f, 0.0f, 1.0f, 0.0f
        };
        vk::Buffer const transform_matrix_buffer = std::make_unique<vkb::core::Buffer>(get_device(), sizeof(transform_matrix), buffer_usage_flags, VMA_MEMORY_USAGE_CPU_TO_GPU);
        transform_matrix_buffer->update(&transform_matrix, sizeof(transform_matrix));

        vk::DeviceOrHostAddressConstKHR const vertex_data_device_address
        {
            .deviceAddress = device.getBufferDeviceAddressKHR({.buffer = vertex_buffer}),
        };
        
        vk::DeviceOrHostAddressConstKHR const index_data_device_address
        {
            .deviceAddress = device.getBufferDeviceAddressKHR({.buffer = index_buffer}),
        };
        
        vk::DeviceOrHostAddressConstKHR const transform_matrix_device_address
        {
            .deviceAddress = device.getBufferDeviceAddressKHR({.buffer = transform_matrix_buffer}),
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
                    .vertexFormat  = vk::Format::eR32G32B32Sfloat,
                    .vertexData = vertex_data_device_address,
                    .maxVertex = static_cast<std::uint32_t>(vertices.size()),
                    .vertexStride = sizeof(Vertex),
                    .indexType = vk::IndexType::eUint32,
                    .indexData = index_data_device_address,
                    .transformData = transform_matrix_device_address,
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
        vk::Buffer const bottom_level_acceleration_structure_buffer = std::make_unique<vkb::core::Buffer>(
            get_device(),
            acceleration_structure_build_sizes_info.accelerationStructureSize,
            vk::BufferUsageFlagBits::eAccelerationStructureStorageKHR,
            VMA_MEMORY_USAGE_GPU_ONLY);

        // Create the acceleration structure
        vk::AccelerationStructureCreateInfoKHR const acceleration_structure_create_info
        {
            .buffer = bottom_level_acceleration_structure_buffer,
            .size = acceleration_structure_build_sizes_info.accelerationStructureSize,
            .type = vk::AccelerationStructureTypeKHR::eBottomLevel,
        };

        vk::AccelerationStructureKHR const bottom_level_acceleration_structure = device.createAccelerationStructureKHR(
            acceleration_structure_create_info,
            allocation_callbacks
        );

        // The actual build process starts here

        // Create a scratch buffer as a temporary storage for the acceleration structure build
        Scratch_buffer const scratch_buffer = create_scratch_buffer(acceleration_structure_build_sizes_info.buildScratchSize, allocation_callbacks);

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
                .deviceAddress = scratch_buffer.device_address,
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
        
        // TODO Create command buffer
        command_buffer.buildAccelerationStructuresKHR(
            1,
            &acceleration_build_geometry_info,
            acceleration_build_structure_range_infos.data()
        );
        // TODO Submit command buffer

        // Get the bottom acceleration structure's handle, which will be used during the top level acceleration build
        std::uint64_t const device_address = device.getAccelerationStructureDeviceAddressKHR({.accelerationStructure = bottom_level_acceleration_structure});

        return
        {
            .handle = bottom_level_acceleration_structure,
            .device_address = device_address,
            .buffer = bottom_level_acceleration_structure_buffer,
        };
    }

    Acceleration_structure create_top_level_acceleration_structure(
        Acceleration_structure const bottom_level_acceleration_structure
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

        vk::Buffer const instances_buffer = std::make_unique<vkb::core::Buffer>(
            get_device(),
            sizeof(VkAccelerationStructureInstanceKHR),
            VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
            VMA_MEMORY_USAGE_CPU_TO_GPU
        );
        instances_buffer->update(&acceleration_structure_instance, sizeof(VkAccelerationStructureInstanceKHR));

        vk::DeviceOrHostAddressConstKHR instance_data_device_address
        {
            .deviceAddress = device.getBufferDeviceAddressKHR({.buffer = instances_buffer}),
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
            .pGeometries   = &acceleration_structure_geometry,
        };

        const uint32_t primitive_count = 1;

        vk::AccelerationStructureBuildSizesInfoKHR const acceleration_structure_build_sizes_info =
            device.getAccelerationStructureBuildSizesKHR(
                vk::AccelerationStructureBuildTypeKHR::eDevice,
                acceleration_structure_build_geometry_info,
                primitive_count,
                acceleration_structure_build_sizes_info
            );

        // Create a buffer to hold the acceleration structure
        vk::Buffer const top_level_acceleration_structure_buffer = std::make_unique<vkb::core::Buffer>(
            get_device(),
            acceleration_structure_build_sizes_info.accelerationStructureSize,
            vk::BufferUsageFlagBits::eAccelerationStructureStorageKHR,
            VMA_MEMORY_USAGE_GPU_ONLY);

        // Create the acceleration structure
        vk::AccelerationStructureCreateInfoKHR const acceleration_structure_create_info
        {
            .buffer = top_level_acceleration_structure_buffer,
            .size = acceleration_structure_build_sizes_info.accelerationStructureSize,
            .type = vk::AccelerationStructureTypeKHR::eTopLevel,
        };
        vk::AccelerationStructureKHR const top_level_acceleration_structure = device.createAccelerationStructureKHR(acceleration_structure_create_info, allocation_callbacks);

        // The actual build process starts here

        // Create a scratch buffer as a temporary storage for the acceleration structure build
        Scratch_buffer const scratch_buffer = create_scratch_buffer(acceleration_structure_build_sizes_info.buildScratchSize, allocation_callbacks);

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
                .deviceAddress = scratch_buffer.device_address,
            },
        };

        vk::AccelerationStructureBuildRangeInfoKHR const acceleration_structure_build_range_info
        {
            .primitiveCount = 1,
            .primitiveOffset = 0,
            .firstVertex = 0,
            .transformOffset = 0,
        };
        std::array<vk::AccelerationStructureBuildRangeInfoKHR*, 1> const acceleration_build_structure_range_infos = {&acceleration_structure_build_range_info};

        // Build the acceleration structure on the device via a one-time command buffer submission
        // Some implementations may support acceleration structure building on the host (VkPhysicalDeviceAccelerationStructureFeaturesKHR->accelerationStructureHostCommands), but we prefer device builds

        // TODO Create command buffer
        command_buffer.buildAccelerationStructuresKHR(
            1,
            &acceleration_build_geometry_info,
            acceleration_build_structure_range_infos.data()
        );
        // TODO Submit command buffer

        // Get the top acceleration structure's handle, which will be used to setup it's descriptor
        std::uint64_t const device_address = device.getAccelerationStructureDeviceAddressKHR({.accelerationStructure = top_level_acceleration_structure});

        return
        {
            .handle = top_level_acceleration_structure,
            .device_address = device_address,
            .buffer = top_level_acceleration_structure_buffer,
        };
    }
}
