export module maia.renderer.vulkan.shader_module;

import maia.renderer.vulkan.allocation_callbacks;
import maia.renderer.vulkan.device;

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
        Device device,
        VkShaderModuleCreateFlags flags,
        std::span<uint32_t const> code,
        std::optional<Allocation_callbacks> allocator = {}) noexcept;

    export void destroy_shader_module(
        Device device,
        Shader_module shader_module,
        std::optional<Allocation_callbacks> allocator = {}) noexcept;
}
