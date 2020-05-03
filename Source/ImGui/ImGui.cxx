export module mythology.imgui;

import maia.renderer.vulkan;

import </home/jpmmaia/Desktop/source/vcpkg/installed/x64-linux/include/imgui.h>; // TODO fix stupid compilation error

import <vulkan/vulkan.h>;

import <cstdint>;

namespace Mythology::ImGui
{
    export struct ImGui_resources
    {
        VkDevice device;
        VkAllocationCallbacks const* allocator;
        Maia::Renderer::Vulkan::Buffer_pool_node geometry_buffer_node;
        VkImage fonts_image;
        VkImageView fonts_image_view;
        VkSampler fonts_sampler;
        VkDescriptorSetLayout descriptor_set_layout;
        VkDescriptorPool descriptor_pool;
        VkDescriptorSet descriptor_set;
        VkPipelineLayout pipeline_layout;
        VkPipeline pipeline;
        Maia::Renderer::Vulkan::Buffer_pool_memory_resource* buffer_pool;


        ImGui_resources(
            VkPhysicalDeviceMemoryProperties const& physical_device_memory_properties,
            VkDevice const device,
            VkDescriptorPool const descriptor_pool,
            VkRenderPass const render_pass,
            std::uint32_t const subpass_index,
            VkShaderModule const vertex_shader_module,
            VkShaderModule const fragment_shader_module,
            Maia::Renderer::Vulkan::Monotonic_device_memory_resource& monotonic_memory_resource,
            Maia::Renderer::Vulkan::Buffer_pool_memory_resource& buffer_pool,
            VkPipelineCache const pipeline_cache = VK_NULL_HANDLE,
            VkAllocationCallbacks const* const allocator = nullptr
        ) noexcept;
        ImGui_resources(ImGui_resources const&) noexcept = delete;
        ImGui_resources(ImGui_resources&& other) noexcept;
        ~ImGui_resources() noexcept;

        ImGui_resources& operator=(ImGui_resources const&) noexcept = delete;
        ImGui_resources& operator=(ImGui_resources&& other) noexcept;
    };

    export void upload_fonts_image_data(
        VkCommandBuffer command_buffer,
        VkImage fonts_image
    ) noexcept;

    export Maia::Renderer::Vulkan::Buffer_pool_node update_geometry_buffer(
        VkDevice device,
        ImDrawData const& draw_data,
        Maia::Renderer::Vulkan::Buffer_pool_node geometry_buffer_node,
        Maia::Renderer::Vulkan::Buffer_pool_memory_resource& geometry_buffer_pool
    ) noexcept;

    export struct Buffer_range
    {
        VkBuffer buffer;
        VkDeviceSize offset;
        VkDeviceSize size;
    };

    export void render(
        ImDrawData const& draw_data,
        VkCommandBuffer command_buffer,
        VkPipeline pipeline,
        VkPipelineLayout pipeline_layout,
        VkDescriptorSet descriptor_set,
        Buffer_range vertex_buffer_range,
        Buffer_range index_buffer_range
    ) noexcept;
}