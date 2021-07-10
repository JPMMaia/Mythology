module;

#include <vulkan/vulkan.hpp>

export module mythology.geometry;

import maia.renderer.vulkan.buffer_resources;

namespace Mythology
{
    export struct Acceleration_structure
    {
        vk::AccelerationStructureKHR handle = {};
        vk::DeviceAddress device_address = 0;
        Maia::Renderer::Vulkan::Buffer_view buffer_view = {};
    };

    export Acceleration_structure create_bottom_level_acceleration_structure(
        vk::Device device,
        vk::CommandBuffer command_buffer,
        Maia::Renderer::Vulkan::Buffer_resources& acceleration_structure_build_input_buffer_resources,
        Maia::Renderer::Vulkan::Buffer_resources& acceleration_structure_storage_buffer_resources,
        Maia::Renderer::Vulkan::Buffer_resources& scratch_buffer_resources,
        vk::AllocationCallbacks const* allocation_callbacks
    );

    export Acceleration_structure create_top_level_acceleration_structure(
        vk::Device const device,
        vk::CommandBuffer const command_buffer,
        Acceleration_structure const bottom_level_acceleration_structure,
        Maia::Renderer::Vulkan::Buffer_resources& acceleration_structure_build_input_buffer_resources,
        Maia::Renderer::Vulkan::Buffer_resources& acceleration_structure_storage_buffer_resources,
        Maia::Renderer::Vulkan::Buffer_resources& scratch_buffer_resources,
        vk::AllocationCallbacks const* allocation_callbacks
    );
}
