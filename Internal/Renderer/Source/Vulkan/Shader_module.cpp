module maia.renderer.vulkan.shader_module;

import maia.renderer.vulkan.check;

import <vulkan/vulkan.h>;

import <memory_resource>;
import <optional>;
import <span>;
import <vector>;

namespace Maia::Renderer::Vulkan
{
    VkShaderModule create_shader_module(
        VkDevice const device,
        VkShaderModuleCreateFlags const flags,
        std::span<uint32_t const> const code,
        VkAllocationCallbacks const* const allocator) noexcept
    {
        VkShaderModuleCreateInfo const create_info
        {
            .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
            .pNext = nullptr,
            .flags = flags,
            .codeSize = code.size_bytes(),
            .pCode = code.data()
        };

        VkShaderModule shader_module = {};
        check_result(
            vkCreateShaderModule(
                device,
                &create_info,
                allocator,
                &shader_module
            )
        );

        return {shader_module};
    }

    void destroy_shader_module(
        VkDevice const device,
        VkShaderModule const shader_module,
        VkAllocationCallbacks const* const allocator) noexcept
    {
        vkDestroyShaderModule(
            device,
            shader_module,
            allocator
        );
    }
}
