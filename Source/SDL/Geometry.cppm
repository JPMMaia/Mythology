module;

#include <vulkan/vulkan.hpp>

#include <memory_resource>
#include <span>
#include <vector>

export module mythology.geometry;

import maia.renderer.vulkan.buffer_resources;
import maia.scene;

namespace Mythology
{
    export struct Acceleration_structure
    {
        vk::AccelerationStructureKHR handle = {};
        vk::DeviceAddress device_address = 0;
        Maia::Renderer::Vulkan::Buffer_view buffer_view = {};
    };

    export std::pmr::vector<Acceleration_structure> create_bottom_level_acceleration_structures(
        vk::PhysicalDeviceType physical_device_type,
        vk::Device device,
        vk::Queue queue,
        vk::CommandPool command_pool,
        Maia::Renderer::Vulkan::Buffer_resources& acceleration_structure_storage_buffer_resources,
        Maia::Renderer::Vulkan::Buffer_resources& geometry_buffer_resources,
        Maia::Renderer::Vulkan::Buffer_resources& upload_buffer_resources,
        Maia::Renderer::Vulkan::Buffer_resources& scratch_buffer_resources,
        Maia::Scene::World const& world,
        std::span<std::pmr::vector<std::byte> const> buffers_data,
        vk::AllocationCallbacks const* allocation_callbacks,
        std::pmr::polymorphic_allocator<> const& output_allocator,
        std::pmr::polymorphic_allocator<> const& temporaries_allocator
    );

    export std::pmr::vector<Acceleration_structure> create_top_level_acceleration_structures(
        vk::PhysicalDeviceType physical_device_type,
        vk::Device device,
        vk::Queue queue,
        vk::CommandPool command_pool,
        std::span<Acceleration_structure const> bottom_level_acceleration_structures,
        Maia::Renderer::Vulkan::Buffer_resources& acceleration_structure_storage_buffer_resources, // vk::BufferUsageFlagBits::eAccelerationStructureStorageKHR
        Maia::Renderer::Vulkan::Buffer_resources& instance_buffer_resources, // vk::BufferUsageFlagBits::eAccelerationStructureBuildInputReadOnlyKHR | vk::BufferUsageFlagBits::eShaderDeviceAddressKHR
        Maia::Renderer::Vulkan::Buffer_resources& upload_buffer_resources,
        Maia::Renderer::Vulkan::Buffer_resources& scratch_buffer_resources, // vk::BufferUsageFlagBits::eStorageBuffer | vk::BufferUsageFlagBits::eShaderDeviceAddressKHR
        Maia::Scene::World const& world,
        Maia::Scene::Scene const& scene,
        vk::AllocationCallbacks const* allocation_callbacks,
        std::pmr::polymorphic_allocator<> const& output_allocator,
        std::pmr::polymorphic_allocator<> const& temporaries_allocator
    );
}
