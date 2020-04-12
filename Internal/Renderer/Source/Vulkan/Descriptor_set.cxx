export module maia.renderer.vulkan.descriptor_set;

import <vulkan/vulkan.h>;

import <array>;
import <cassert>;
import <cstddef>;
import <memory_resource>;
import <span>;
import <vector>;

namespace Maia::Renderer::Vulkan
{
    export VkDescriptorSet allocate_descriptor_set(
        VkDevice device,
        VkDescriptorSetAllocateInfo const& allocate_info
    ) noexcept;

    export std::pmr::vector<VkDescriptorSet> allocate_descriptor_sets(
        VkDevice device,
        VkDescriptorSetAllocateInfo const& allocate_info,
        std::pmr::polymorphic_allocator<VkDescriptorSet> vector_allocator = {}
    ) noexcept;


    export template <std::size_t count>
    std::array<VkDescriptorSet, count> allocate_descriptor_sets(
        VkDevice const device,
        VkDescriptorSetAllocateInfo const& allocate_info
    ) noexcept
    {
        assert(allocate_info.descriptorSetCount == count);

        std::array<VkDescriptorSet, count> descriptor_sets{};

        check_result(
            vkAllocateDescriptorSets(device, &allocate_info, descriptor_sets.data()));

        return descriptor_sets;
    }

    export void free_descriptor_sets(
        VkDevice device,
        VkDescriptorPool descriptor_pool,
        std::span<VkDescriptorSet const> descriptor_sets
    ) noexcept;
}
