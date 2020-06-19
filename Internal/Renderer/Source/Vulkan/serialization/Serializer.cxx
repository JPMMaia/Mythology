export module maia.renderer.vulkan.serializer;

import <nlohmann/json.hpp>;
import <vulkan/vulkan.h>;

import <cstddef>;
import <memory_resource>;
import <vector>;

namespace Maia::Renderer::Vulkan
{
    export struct Commands_data
    {
        std::pmr::vector<std::byte> bytes;
    };

    export Commands_data create_commands_data(
        nlohmann::json const& commands_json,
        std::pmr::polymorphic_allocator<std::byte> const& commands_allocator
    ) noexcept;

    export void draw(
        VkCommandBuffer const command_buffer,
        VkImage const output_image,
        VkImageSubresourceRange const& output_image_subresource_range,
        Commands_data const& commands_data
    ) noexcept;
}
