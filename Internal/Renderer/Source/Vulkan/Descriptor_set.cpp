module maia.renderer.vulkan.descriptor_set;

import maia.renderer.vulkan.check;

import <vulkan/vulkan.h>;

import <array>;
import <cassert>;
import <memory_resource>;
import <span>;
import <vector>;

namespace Maia::Renderer::Vulkan
{
    VkDescriptorSet allocate_descriptor_set(
        VkDevice const device,
        VkDescriptorSetAllocateInfo const& allocate_info
    ) noexcept
    {
        assert(allocate_info.descriptorSetCount == 1);

        VkDescriptorSet descriptor_set{};

        check_result(
            vkAllocateDescriptorSets(device, &allocate_info, &descriptor_set));

        return descriptor_set;
    }

    std::pmr::vector<VkDescriptorSet> allocate_descriptor_sets(
        VkDevice const device,
        VkDescriptorSetAllocateInfo const& allocate_info,
        std::pmr::polymorphic_allocator<VkDescriptorSet> vector_allocator
    ) noexcept
    {
        std::pmr::vector<VkDescriptorSet> descriptor_sets{std::move(vector_allocator)};
        descriptor_sets.resize(allocate_info.descriptorSetCount, VK_NULL_HANDLE);

        check_result(
            vkAllocateDescriptorSets(device, &allocate_info, descriptor_sets.data()));

        return descriptor_sets;
    }

    void free_descriptor_sets(
        VkDevice const device,
        VkDescriptorPool const descriptor_pool,
        std::span<VkDescriptorSet const> const descriptor_sets
    ) noexcept
    {
        check_result(
            vkFreeDescriptorSets(device, descriptor_pool, descriptor_sets.size(), descriptor_sets.data()));
    }
}
