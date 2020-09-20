export module maia.renderer.vulkan.shader_module;


import <vulkan/vulkan.h>;

import <memory_resource>;
import <optional>;
import <span>;
import <vector>;

namespace Maia::Renderer::Vulkan
{
    export struct Shader_module
    {
        VkShaderModule value;
    };

    export Shader_module create_shader_module(
        VkDevice device,
        VkShaderModuleCreateFlags flags,
        std::span<uint32_t const> code,
        VkAllocationCallbacks const* allocator = {}) noexcept;

    export void destroy_shader_module(
        VkDevice device,
        Shader_module shader_module,
        VkAllocationCallbacks const* allocator = {}) noexcept;
}
