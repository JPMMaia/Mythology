module;

#include <vulkan/vulkan.h>

#include <cassert>
#include <stdexcept>

module maia.renderer.vulkan.check;

namespace Maia::Renderer::Vulkan
{
    void check_result(VkResult const result)
    {
        if (result != VK_SUCCESS)
        {
            throw std::runtime_error{"Vulkan error!"};
        }
    }
}
