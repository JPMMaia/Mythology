module maia.renderer.vulkan.shader_module;

import maia.renderer.vulkan.check;
import maia.renderer.vulkan.device;

import <vulkan/vulkan.h>;

import <memory_resource>;
import <optional>;
import <span>;
import <vector>;

namespace Maia::Renderer::Vulkan
{
    Shader_module create_shader_module(
        Device const device,
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
                device.value,
                &create_info,
                allocator,
                &shader_module
            )
        );

        return {shader_module};
    }

    void destroy_shader_module(
        Device const device,
        Shader_module const shader_module,
        VkAllocationCallbacks const* const allocator) noexcept
    {
        vkDestroyShaderModule(
            device.value,
            shader_module.value,
            allocator
        );
    }
}
