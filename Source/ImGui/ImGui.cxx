export module mythology.imgui;

import maia.renderer.vulkan.memory_management;

import <vulkan/vulkan.h>;

import <cstdint>;

namespace Mythology::ImGui
{
    export struct ImGui_resources
    {
        VkDevice device;
        VkAllocationCallbacks const* allocator;
        VkImage fonts_image;
        VkImageView fonts_image_view;
        VkSampler fonts_sampler;
        VkDescriptorSetLayout descriptor_set_layout;
        VkDescriptorPool descriptor_pool;
        VkDescriptorSet descriptor_set;
        VkPipelineLayout pipeline_layout;
        VkPipeline pipeline;


        ImGui_resources(
            VkPhysicalDeviceMemoryProperties const& physical_device_memory_properties,
            VkDevice const device,
            VkDescriptorPool const descriptor_pool,
            VkRenderPass const render_pass,
            std::uint32_t const subpass_index,
            VkShaderModule const vertex_shader_module,
            VkShaderModule const fragment_shader_module,
            Maia::Renderer::Vulkan::Monotonic_device_memory_resource& monotonic_memory_resource,
            VkPipelineCache const pipeline_cache = VK_NULL_HANDLE,
            VkAllocationCallbacks const* const allocator = nullptr
        ) noexcept;
        ImGui_resources(ImGui_resources const&) noexcept = delete;
        ImGui_resources(ImGui_resources&& other) noexcept;
        ~ImGui_resources() noexcept;

        ImGui_resources& operator=(ImGui_resources const&) noexcept = delete;
        ImGui_resources& operator=(ImGui_resources&& other) noexcept;
    };
}