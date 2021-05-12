module;

#include <vulkan/vulkan.h>

export module maia.renderer.vulkan.check;

namespace Maia::Renderer::Vulkan
{
    export void check_result(VkResult const result);
}
