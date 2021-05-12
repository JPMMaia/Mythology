module;

#include <vulkan/vulkan.h>

#include <memory_resource>
#include <optional>
#include <span>
#include <vector>

export module maia.renderer.vulkan.shader_module;
namespace Maia::Renderer::Vulkan
{
    export VkShaderModule create_shader_module(
        VkDevice device,
        VkShaderModuleCreateFlags flags,
        std::span<uint32_t const> code,
        VkAllocationCallbacks const* allocator = {}) noexcept;

    export void destroy_shader_module(
        VkDevice device,
        VkShaderModule shader_module,
        VkAllocationCallbacks const* allocator = {}) noexcept;
}
