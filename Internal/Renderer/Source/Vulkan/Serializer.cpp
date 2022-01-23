module;

#include <nlohmann/json.hpp>
#include <vulkan/vulkan.hpp>

#include <cstddef>
#include <filesystem>
#include <fstream>
#include <memory_resource>
#include <optional>
#include <span>
#include <string>
#include <vector>

module maia.renderer.vulkan.serializer;

import maia.renderer.vulkan.buffer_resources;
import maia.renderer.vulkan.image_resources;
import maia.renderer.vulkan.upload;

namespace nlohmann
{
    template <>
    struct adl_serializer<vk::BlendFactor>
    {
        static void to_json(json& j, vk::BlendFactor const& value)
        {
            assert(false);
        }

        static void from_json(const json& j, vk::BlendFactor& value)
        {
            value = static_cast<vk::BlendFactor>(j.get<std::uint32_t>());
        }
    };

    template <>
    struct adl_serializer<vk::ComponentMapping>
    {
        static void to_json(json& j, vk::ComponentMapping const& value)
        {
            assert(false);
        }

        static void from_json(const json& j, vk::ComponentMapping& value)
        {
            value = vk::ComponentMapping
            {
                .r = static_cast<vk::ComponentSwizzle>(j.at("r").get<std::uint32_t>()),
                .g = static_cast<vk::ComponentSwizzle>(j.at("g").get<std::uint32_t>()),
                .b = static_cast<vk::ComponentSwizzle>(j.at("b").get<std::uint32_t>()),
                .a = static_cast<vk::ComponentSwizzle>(j.at("a").get<std::uint32_t>()),
            };
        }
    };

    template <typename BitType>
    struct adl_serializer<vk::Flags<BitType>>
    {
        static void to_json(json& j, vk::Flags<BitType> const& value)
        {
            assert(false);
        }

        static void from_json(const json& j, vk::Flags<BitType>& value)
        {
            BitType const bit = j.get<BitType>();
            value = vk::Flags<BitType>(bit);
        }
    };

    template <>
    struct adl_serializer<vk::Extent3D>
    {
        static void to_json(json& j, vk::Extent3D const& value)
        {
            assert(false);
        }

        static void from_json(const json& j, vk::Extent3D& value)
        {
            value = vk::Extent3D
            {
                .width = j.at("width").get<std::uint32_t>(),
                .height = j.at("height").get<std::uint32_t>(),
                .depth = j.at("depth").get<std::uint32_t>(),
            };
        }
    };

    template <>
    struct adl_serializer<vk::ImageSubresourceRange>
    {
        static void to_json(json& j, vk::ImageSubresourceRange const& value)
        {
            assert(false);
        }

        static void from_json(const json& j, vk::ImageSubresourceRange& value)
        {
            value = vk::ImageSubresourceRange
            {
                .aspectMask = j.at("aspect_mask").get<vk::ImageAspectFlags>(),
                .baseMipLevel = j.at("base_mip_level").get<std::uint32_t>(),
                .levelCount = j.at("level_count").get<std::uint32_t>(),
                .baseArrayLayer = j.at("base_array_layer").get<std::uint32_t>(),
                .layerCount = j.at("layer_count").get<std::uint32_t>(),
            };
        }
    };
}

namespace Maia::Renderer::Vulkan
{
    namespace
    {
        Maia::Renderer::Vulkan::Buffer_resources create_buffer_resources(
            nlohmann::json const& buffers_json,
            vk::PhysicalDevice const physical_device,
            vk::Device const device,
            vk::PhysicalDeviceType const physical_device_type,
            vk::AllocationCallbacks const* const allocation_callbacks,
            std::pmr::polymorphic_allocator<> const& allocator
        )
        {
            vk::DeviceSize const block_size = 64 * 1024 * 1024;

            vk::BufferUsageFlags const usage_flags = [&]() -> vk::BufferUsageFlags
            {
                vk::BufferUsageFlags usage_flags = {};

                for (nlohmann::json const& buffer_json : buffers_json)
                {
                    usage_flags |= buffer_json.at("usage").get<vk::BufferUsageFlags>();
                }

                return usage_flags;
            }();

            return Buffer_resources
            {
                physical_device,
                device,
                physical_device_type,
                {},
                block_size,
                usage_flags,
                {},
                {},
                allocation_callbacks,
                allocator
            };
        }

        std::pmr::vector<Maia::Renderer::Vulkan::Buffer_view> create_buffers(
            nlohmann::json const& buffers_json,
            Maia::Renderer::Vulkan::Buffer_resources& buffer_resources,
            std::pmr::polymorphic_allocator<> const& output_allocator
        )
        {
            std::pmr::vector<Maia::Renderer::Vulkan::Buffer_view> buffers{ output_allocator };
            buffers.reserve(buffers_json.size());

            for (nlohmann::json const& buffer_json : buffers_json)
            {
                vk::DeviceSize const size = buffer_json.at("size").get<vk::DeviceSize>();
                buffers.push_back(buffer_resources.allocate_buffer(size, 1, vk::MemoryPropertyFlagBits::eDeviceLocal));
            }

            return buffers;
        }

        std::pmr::vector<vk::BufferView> create_buffer_views(
            nlohmann::json const& buffer_views_json,
            vk::Device const device,
            std::span<Maia::Renderer::Vulkan::Buffer_view const> const buffers,
            vk::AllocationCallbacks const* const allocator,
            std::pmr::polymorphic_allocator<> const& output_allocator
        )
        {
            std::pmr::vector<vk::BufferView> buffer_views{ output_allocator };
            buffer_views.reserve(buffer_views_json.size());

            for (nlohmann::json const& buffer_view_json : buffer_views_json)
            {
                std::size_t const buffer_index = buffer_view_json.at("buffer").get<std::size_t>();
                vk::Format const format = buffer_view_json.at("format").get<vk::Format>();
                vk::DeviceSize const offset = buffer_view_json.at("offset").get<vk::DeviceSize>();
                vk::DeviceSize const range = buffer_view_json.at("range").get<vk::DeviceSize>();

                Maia::Renderer::Vulkan::Buffer_view const buffer = buffers[buffer_index];

                vk::BufferViewCreateInfo const create_info
                {
                    .flags = {},
                    .buffer = buffer.buffer,
                    .format = format,
                    .offset = buffer.offset + offset,
                    .range = range,
                };

                vk::BufferView const buffer_view = device.createBufferView(create_info, allocator);

                buffer_views.push_back(buffer_view);
            }

            return buffer_views;
        }

        std::pmr::vector<Maia::Renderer::Vulkan::Image_memory_view> create_images(
            nlohmann::json const& images_json,
            Maia::Renderer::Vulkan::Image_resources& image_resources,
            std::pmr::polymorphic_allocator<> const& output_allocator
        )
        {
            std::pmr::vector<Maia::Renderer::Vulkan::Image_memory_view> images{ output_allocator };
            images.reserve(images_json.size());

            for (nlohmann::json const& image_json : images_json)
            {
                vk::ImageCreateInfo const create_info
                {
                    .flags = image_json.at("flags").get<vk::ImageCreateFlags>(),
                    .imageType = image_json.at("type").get<vk::ImageType>(),
                    .format = image_json.at("format").get<vk::Format>(),
                    .extent = image_json.at("extent").get<vk::Extent3D>(),
                    .mipLevels = image_json.at("mipLevels").get<std::uint32_t>(),
                    .arrayLayers = image_json.at("arrayLayers").get<std::uint32_t>(),
                    .samples = image_json.at("samples").get<vk::SampleCountFlagBits>(),
                    .tiling = image_json.at("tiling").get<vk::ImageTiling>(),
                    .usage = image_json.at("usage").get<vk::ImageUsageFlags>(),
                    .sharingMode = vk::SharingMode::eExclusive,
                    .queueFamilyIndexCount = 0,
                    .pQueueFamilyIndices = nullptr,
                    .initialLayout = image_json.at("initial_layout").get<vk::ImageLayout>(),
                };

                Maia::Renderer::Vulkan::Image_memory_view const image_memory_view =
                    image_resources.allocate_image(create_info, vk::MemoryPropertyFlagBits::eDeviceLocal);

                images.push_back(image_memory_view);
            }

            return images;
        }

        std::pmr::vector<vk::ImageView> create_image_views(
            nlohmann::json const& image_views_json,
            vk::Device const device,
            std::span<Maia::Renderer::Vulkan::Image_memory_view const> const images,
            vk::AllocationCallbacks const* const allocator,
            std::pmr::polymorphic_allocator<> const& output_allocator
        )
        {
            std::pmr::vector<vk::ImageView> image_views{ output_allocator };
            image_views.reserve(image_views_json.size());

            for (nlohmann::json const& image_view_json : image_views_json)
            {
                vk::ImageViewCreateFlags const flags = image_view_json.at("flags").get<vk::ImageViewCreateFlags>();
                std::size_t const image_index = image_view_json.at("image").get<std::size_t>();
                vk::ImageViewType const view_type = image_view_json.at("view_type").get<vk::ImageViewType>();
                vk::Format const format = image_view_json.at("format").get<vk::Format>();
                vk::ComponentMapping const components = image_view_json.at("components").get<vk::ComponentMapping>();
                vk::ImageSubresourceRange const subresource_range = image_view_json.at("subresource_range").get<vk::ImageSubresourceRange>();

                Maia::Renderer::Vulkan::Image_memory_view const image_memory_view = images[image_index];

                vk::ImageViewCreateInfo const create_info
                {
                    .flags = flags,
                    .image = image_memory_view.image,
                    .viewType = view_type,
                    .format = format,
                    .components = components,
                    .subresourceRange = subresource_range,
                };

                vk::ImageView const image_view = device.createImageView(create_info, allocator);

                image_views.push_back(image_view);
            }

            return image_views;
        }
    }

    vk::DescriptorPool create_descriptor_pool(
        nlohmann::json const& descriptor_sets_json,
        nlohmann::json const& descriptor_set_layouts_json,
        vk::Device const device,
        std::uint32_t const descriptor_set_count_multiplier,
        vk::AllocationCallbacks const* const allocation_callbacks,
        std::pmr::polymorphic_allocator<> const& temporaries_allocator
    )
    {
        std::pmr::vector<vk::DescriptorPoolSize> pool_sizes{ temporaries_allocator };
        pool_sizes.reserve(15);

        for (nlohmann::json const& descriptor_set_json : descriptor_sets_json)
        {
            std::size_t const descriptor_set_layout_index = descriptor_set_json.at("layout").get<std::size_t>();
            nlohmann::json const& descriptor_set_layout_json = descriptor_set_layouts_json[descriptor_set_layout_index];

            for (nlohmann::json const& binding_json : descriptor_set_layout_json.at("bindings"))
            {
                vk::DescriptorType const descriptor_type = binding_json.at("descriptor_type").get<vk::DescriptorType>();
                std::uint32_t const descriptor_count = binding_json.at("descriptor_count").get<std::uint32_t>();

                auto const pool_size_iterator =
                    std::find_if(pool_sizes.begin(), pool_sizes.end(), [descriptor_type](vk::DescriptorPoolSize const& size) -> bool { return size.type == descriptor_type; });

                if (pool_size_iterator == pool_sizes.end())
                {
                    pool_sizes.push_back(
                        vk::DescriptorPoolSize
                        {
                            .type = descriptor_type,
                            .descriptorCount = descriptor_count,
                        }
                    );
                }
                else
                {
                    pool_size_iterator->descriptorCount += descriptor_count;
                }
            }
        }

        vk::DescriptorPoolCreateInfo const create_info
        {
            .flags = {},
            .maxSets = static_cast<std::uint32_t>(descriptor_sets_json.size()) * descriptor_set_count_multiplier,
            .poolSizeCount = static_cast<std::uint32_t>(pool_sizes.size()),
            .pPoolSizes = pool_sizes.data(),
        };

        return device.createDescriptorPool(create_info, allocation_callbacks);
    }

    std::pmr::vector<vk::DescriptorSet> create_descriptor_sets(
        nlohmann::json const& descriptor_sets_json,
        vk::Device const device,
        vk::DescriptorPool const descriptor_pool,
        std::span<vk::DescriptorSetLayout const> const descriptor_set_layouts,
        std::pmr::polymorphic_allocator<> const& output_allocator,
        std::pmr::polymorphic_allocator<> const& temporaries_allocator
    )
    {
        std::pmr::vector<vk::DescriptorSetLayout> ordered_descriptor_set_layouts{ temporaries_allocator };
        ordered_descriptor_set_layouts.reserve(descriptor_sets_json.size());

        for (nlohmann::json const& descriptor_set_json : descriptor_sets_json)
        {
            std::size_t const descriptor_set_layout_index = descriptor_set_json.at("layout").get<std::size_t>();

            ordered_descriptor_set_layouts.push_back(descriptor_set_layouts[descriptor_set_layout_index]);
        }

        vk::DescriptorSetAllocateInfo const allocate_info
        {
            .descriptorPool = descriptor_pool,
            .descriptorSetCount = static_cast<std::uint32_t>(ordered_descriptor_set_layouts.size()),
            .pSetLayouts = ordered_descriptor_set_layouts.data(),
        };

        std::pmr::polymorphic_allocator<vk::DescriptorSet> descriptor_sets_allocator{ output_allocator };
        std::pmr::vector<vk::DescriptorSet> descriptor_sets =
            device.allocateDescriptorSets(allocate_info, descriptor_sets_allocator);

        return descriptor_sets;
    }

    void update_descriptor_sets(
        nlohmann::json const& descriptor_sets_json,
        vk::Device const device,
        std::span<vk::DescriptorSet const> const descriptor_sets,
        std::span<Maia::Renderer::Vulkan::Buffer_view const> const buffers,
        std::span<vk::BufferView const> const buffer_views,
        std::span<vk::ImageView const> const image_views,
        std::span<vk::Sampler const> const samplers,
        std::span<vk::AccelerationStructureKHR const> const acceleration_structures,
        std::pmr::polymorphic_allocator<> const& temporaries_allocator
    )
    {
        // TODO reserve
        std::pmr::vector<vk::DescriptorBufferInfo> buffer_infos{ temporaries_allocator };
        std::pmr::vector<vk::DescriptorImageInfo> image_infos{ temporaries_allocator };
        std::pmr::vector<vk::BufferView> ordered_buffer_views{ temporaries_allocator };
        std::pmr::vector<vk::AccelerationStructureKHR> ordered_acceleration_structures{ temporaries_allocator };
        std::pmr::vector<vk::WriteDescriptorSetAccelerationStructureKHR> acceleration_structure_writes{ temporaries_allocator };
        std::pmr::vector<vk::WriteDescriptorSet> writes{ temporaries_allocator };

        for (std::size_t descriptor_set_index = 0; descriptor_set_index < descriptor_sets_json.size(); ++descriptor_set_index)
        {
            nlohmann::json const& descriptor_set_json = descriptor_sets_json[descriptor_set_index];
            nlohmann::json const& bindings_json = descriptor_set_json.at("bindings");

            vk::DescriptorSet const descriptor_set = descriptor_sets[descriptor_set_index];

            for (nlohmann::json const& binding_json : bindings_json)
            {
                vk::DescriptorType const descriptor_type = binding_json.at("descriptor_type").get<vk::DescriptorType>();

                switch (descriptor_type)
                {
                case vk::DescriptorType::eSampler:
                case vk::DescriptorType::eCombinedImageSampler:
                case vk::DescriptorType::eSampledImage:
                case vk::DescriptorType::eStorageImage:
                case vk::DescriptorType::eInputAttachment:
                {
                    nlohmann::json const& image_infos_json = binding_json.at("image_infos");

                    for (nlohmann::json const& image_info_json : image_infos_json)
                    {
                        if (descriptor_type == vk::DescriptorType::eSampler)
                        {
                            std::size_t const sampler_index = image_info_json.at("sampler").get<std::size_t>();

                            image_infos.push_back(
                                vk::DescriptorImageInfo
                                {
                                    .sampler = samplers[sampler_index],
                                }
                            );
                        }
                        else if (descriptor_type == vk::DescriptorType::eCombinedImageSampler)
                        {
                            std::size_t const sampler_index = image_info_json.at("sampler").get<std::size_t>();
                            std::size_t const image_view_index = image_info_json.at("image_view").get<std::size_t>();
                            vk::ImageLayout const image_layout = image_info_json.at("image_layout").get<vk::ImageLayout>();

                            image_infos.push_back(
                                vk::DescriptorImageInfo
                                {
                                    .sampler = samplers[sampler_index],
                                    .imageView = image_views[image_view_index],
                                    .imageLayout = image_layout,
                                }
                            );
                        }
                        else
                        {
                            std::size_t const image_view_index = image_info_json.at("image_view").get<std::size_t>();
                            vk::ImageLayout const image_layout = image_info_json.at("image_layout").get<vk::ImageLayout>();

                            image_infos.push_back(
                                vk::DescriptorImageInfo
                                {
                                    .imageView = image_views[image_view_index],
                                    .imageLayout = image_layout,
                                }
                            );
                        }
                    }

                    assert(image_infos_json.size() <= image_infos.size());

                    writes.push_back(
                        vk::WriteDescriptorSet
                        {
                            .dstSet = descriptor_set,
                            .dstBinding = binding_json.at("binding").get<std::uint32_t>(),
                            .dstArrayElement = binding_json.at("first_array_element").get<std::uint32_t>(),
                            .descriptorCount = static_cast<std::uint32_t>(image_infos_json.size()),
                            .descriptorType = descriptor_type,
                            .pImageInfo = image_infos.data() + (image_infos.size() - image_infos_json.size()),
                        }
                    );

                    break;
                }
                case vk::DescriptorType::eUniformTexelBuffer:
                case vk::DescriptorType::eStorageTexelBuffer:
                {
                    nlohmann::json const& buffer_views_json = binding_json.at("buffer_views");

                    for (std::size_t const buffer_view_index : buffer_views_json)
                    {
                        ordered_buffer_views.push_back(
                            buffer_views[buffer_view_index]
                        );
                    }

                    assert(buffer_views_json.size() <= ordered_buffer_views.size());

                    writes.push_back(
                        vk::WriteDescriptorSet
                        {
                            .dstSet = descriptor_set,
                            .dstBinding = binding_json.at("binding").get<std::uint32_t>(),
                            .dstArrayElement = binding_json.at("first_array_element").get<std::uint32_t>(),
                            .descriptorCount = static_cast<std::uint32_t>(buffer_views_json.size()),
                            .descriptorType = descriptor_type,
                            .pTexelBufferView = ordered_buffer_views.data() + (ordered_buffer_views.size() - buffer_views_json.size()),
                        }
                    );

                    break;
                }
                case vk::DescriptorType::eUniformBuffer:
                case vk::DescriptorType::eStorageBuffer:
                case vk::DescriptorType::eUniformBufferDynamic:
                case vk::DescriptorType::eStorageBufferDynamic:
                {
                    nlohmann::json const& buffer_infos_json = binding_json.at("buffer_infos");

                    for (nlohmann::json const& buffer_info_json : buffer_infos_json)
                    {
                        std::size_t const buffer_index = buffer_info_json.at("buffer").get<std::size_t>();
                        vk::DeviceSize const offset = buffer_info_json.at("offset").get<vk::DeviceSize>();
                        vk::DeviceSize const range = buffer_info_json.at("range").get<vk::DeviceSize>();

                        Maia::Renderer::Vulkan::Buffer_view const& buffer_memory_view = buffers[buffer_index];

                        if ((offset + range) > buffer_memory_view.size)
                        {
                            throw std::runtime_error{ "DescriptorBufferInfo offset+range out of bounds!" };
                        }

                        buffer_infos.push_back(
                            vk::DescriptorBufferInfo
                            {
                                .buffer = buffer_memory_view.buffer,
                                .offset = buffer_memory_view.offset + offset,
                                .range = range,
                            }
                        );
                    }

                    assert(buffer_infos_json.size() <= buffer_infos.size());

                    writes.push_back(
                        vk::WriteDescriptorSet
                        {
                            .dstSet = descriptor_set,
                            .dstBinding = binding_json.at("binding").get<std::uint32_t>(),
                            .dstArrayElement = binding_json.at("first_array_element").get<std::uint32_t>(),
                            .descriptorCount = static_cast<std::uint32_t>(buffer_infos_json.size()),
                            .descriptorType = descriptor_type,
                            .pBufferInfo = buffer_infos.data() + (buffer_infos.size() - buffer_infos_json.size()),
                        }
                    );

                    break;
                }
                case vk::DescriptorType::eAccelerationStructureKHR:
                {
                    nlohmann::json const& acceleration_structures_json = binding_json.at("acceleration_structures");

                    for (std::size_t const acceleration_structure_index : acceleration_structures_json)
                    {
                        ordered_acceleration_structures.push_back(
                            acceleration_structures[acceleration_structure_index]
                        );
                    }

                    assert(acceleration_structures_json.size() <= ordered_acceleration_structures.size());

                    acceleration_structure_writes.push_back(
                        vk::WriteDescriptorSetAccelerationStructureKHR
                        {
                            .accelerationStructureCount = static_cast<std::uint32_t>(acceleration_structures_json.size()),
                            .pAccelerationStructures = ordered_acceleration_structures.data() + (ordered_acceleration_structures.size() - acceleration_structures_json.size())
                        }
                    );

                    assert(!acceleration_structure_writes.empty());

                    writes.push_back(
                        vk::WriteDescriptorSet
                        {
                            .pNext = acceleration_structure_writes.data() + acceleration_structure_writes.size() - 1,
                            .dstSet = descriptor_set,
                            .dstBinding = binding_json.at("binding").get<std::uint32_t>(),
                            .dstArrayElement = binding_json.at("first_array_element").get<std::uint32_t>(),
                            .descriptorType = descriptor_type,
                        }
                    );

                    break;
                }
                }
            }
        }

        device.updateDescriptorSets(writes, {});
    }

    std::pmr::vector<std::pmr::vector<vk::DescriptorSet>> create_frame_descriptor_sets(
        nlohmann::json const& frame_descriptor_sets_json,
        vk::Device const device,
        vk::DescriptorPool const descriptor_pool,
        std::span<vk::DescriptorSetLayout const> const descriptor_set_layouts,
        std::size_t const frames_in_flight,
        std::pmr::polymorphic_allocator<> const& output_allocator,
        std::pmr::polymorphic_allocator<> const& temporaries_allocator
    )
    {
        std::pmr::vector<std::pmr::vector<vk::DescriptorSet>> descriptor_sets{ output_allocator };
        descriptor_sets.reserve(frames_in_flight);

        for (std::size_t frame_index = 0; frame_index < frames_in_flight; ++frame_index)
        {
            std::pmr::vector<vk::DescriptorSet> frame_descriptor_sets = create_descriptor_sets(
                frame_descriptor_sets_json,
                device,
                descriptor_pool,
                descriptor_set_layouts,
                output_allocator,
                temporaries_allocator
            );

            descriptor_sets.push_back(std::move(frame_descriptor_sets));
        }

        return descriptor_sets;
    }

    std::pmr::vector<std::pmr::vector<Frame_descriptor_set_binding>> create_descriptor_sets_bindings(
        nlohmann::json const& frame_descriptor_sets,
        std::pmr::polymorphic_allocator<> const& output_allocator
    )
    {
        std::pmr::vector<std::pmr::vector<Frame_descriptor_set_binding>> descriptor_sets_bindings{ output_allocator };
        descriptor_sets_bindings.reserve(frame_descriptor_sets.size());

        for (nlohmann::json const& descriptor_set_json : frame_descriptor_sets)
        {
            nlohmann::json const& bindings_json = descriptor_set_json.at("bindings");

            std::pmr::vector<Frame_descriptor_set_binding> descriptor_set_bindings;
            descriptor_set_bindings.reserve(bindings_json.size());

            for (nlohmann::json const& binding_json : bindings_json)
            {
                descriptor_set_bindings.push_back(
                    Frame_descriptor_set_binding
                    {
                        .descriptor_type = binding_json.at("descriptor_type").get<vk::DescriptorType>(),
                        .binding = binding_json.at("binding").get<std::uint32_t>(),
                        .first_array_element = binding_json.at("first_array_element").get<std::uint32_t>(),
                    }
                );
            }

            descriptor_sets_bindings.push_back(std::move(descriptor_set_bindings));
        }

        return descriptor_sets_bindings;
    }

    std::pmr::vector<std::pmr::vector<std::pmr::vector<vk::ImageLayout>>> create_descriptor_sets_image_layouts(
        nlohmann::json const& frame_descriptor_sets,
        std::pmr::polymorphic_allocator<> const& output_allocator
    )
    {
        std::pmr::vector<std::pmr::vector<std::pmr::vector<vk::ImageLayout>>> descriptor_sets_image_layouts{ output_allocator };
        descriptor_sets_image_layouts.reserve(frame_descriptor_sets.size());

        for (nlohmann::json const& descriptor_set_json : frame_descriptor_sets)
        {
            nlohmann::json const& bindings_json = descriptor_set_json.at("bindings");

            std::pmr::vector<std::pmr::vector<vk::ImageLayout>> descriptor_set_image_layouts{ output_allocator };
            descriptor_set_image_layouts.reserve(bindings_json.size());

            for (nlohmann::json const& binding_json : bindings_json)
            {
                nlohmann::json const& image_infos_json = binding_json.at("image_infos");

                std::pmr::vector<vk::ImageLayout> image_layouts{ output_allocator };
                image_layouts.reserve(image_infos_json.size());

                for (nlohmann::json const& image_info_json : image_infos_json)
                {
                    image_layouts.push_back(
                        image_info_json.at("image_layout").get<vk::ImageLayout>()
                    );
                }

                descriptor_set_image_layouts.push_back(std::move(image_layouts));
            }

            descriptor_sets_image_layouts.push_back(std::move(descriptor_set_image_layouts));
        }

        return descriptor_sets_image_layouts;
    }


    std::pmr::vector<std::pmr::vector<std::pmr::vector<std::size_t>>> create_descriptor_sets_image_indices(
        nlohmann::json const& frame_descriptor_sets,
        std::span<std::size_t const> const input_index_to_image_index,
        std::pmr::polymorphic_allocator<> const& output_allocator
    )
    {
        std::pmr::vector<std::pmr::vector<std::pmr::vector<std::size_t>>> image_indices{ output_allocator };
        image_indices.reserve(frame_descriptor_sets.size());

        for (nlohmann::json const& descriptor_set : frame_descriptor_sets)
        {
            nlohmann::json const& bindings = descriptor_set.at("bindings");

            std::pmr::vector<std::pmr::vector<std::size_t>> descriptor_set_indices{ output_allocator };
            descriptor_set_indices.reserve(bindings.size());

            for (nlohmann::json const& binding : bindings)
            {
                nlohmann::json const& image_infos = binding.at("image_infos");

                std::pmr::vector<std::size_t> binding_image_indices{ output_allocator };
                binding_image_indices.reserve(image_infos.size());

                for (nlohmann::json const& image_info : image_infos)
                {
                    std::size_t const input_index = image_info.at("image_view").get<std::size_t>();
                    std::size_t const image_view_index = input_index_to_image_index[input_index];

                    binding_image_indices.push_back(image_view_index);
                }

                descriptor_set_indices.push_back(std::move(binding_image_indices));
            }

            image_indices.push_back(std::move(descriptor_set_indices));
        }

        return image_indices;
    }

    std::pmr::vector<std::pmr::vector<std::pmr::vector<vk::DescriptorImageInfo>>> create_frame_descriptor_set_image_infos(
        std::span<std::pmr::vector<std::pmr::vector<std::size_t>> const> const descriptor_sets_image_indices,
        std::span<std::pmr::vector<std::pmr::vector<vk::ImageLayout>> const> const descriptor_sets_image_layouts,
        std::span<vk::ImageView const> const frame_image_views,
        std::pmr::polymorphic_allocator<> const& output_allocator
    )
    {
        assert(descriptor_sets_image_indices.size() == descriptor_sets_image_layouts.size());

        std::pmr::vector<std::pmr::vector<std::pmr::vector<vk::DescriptorImageInfo>>> image_infos{ output_allocator };

        for (std::size_t descriptor_set_index = 0; descriptor_set_index < descriptor_sets_image_indices.size(); ++descriptor_set_index)
        {
            std::span<std::pmr::vector<std::size_t> const> const descriptor_set_image_indices = descriptor_sets_image_indices[descriptor_set_index];

            std::pmr::vector<std::pmr::vector<vk::DescriptorImageInfo>> descriptor_set_image_infos{ output_allocator };
            descriptor_set_image_infos.reserve(descriptor_set_image_indices.size());

            for (std::size_t binding_index = 0; binding_index < descriptor_set_image_indices.size(); ++binding_index)
            {
                std::span<std::size_t const> const binding_image_indices = descriptor_set_image_indices[binding_index];

                std::pmr::vector<vk::DescriptorImageInfo> binding_image_infos{ output_allocator };
                binding_image_infos.reserve(binding_image_indices.size());

                for (std::size_t image_info_index = 0; image_info_index < binding_image_indices.size(); ++image_info_index)
                {
                    std::size_t const image_index = binding_image_indices[image_info_index];
                    vk::ImageLayout const image_layout = descriptor_sets_image_layouts[descriptor_set_index][binding_index][image_info_index];

                    binding_image_infos.push_back(
                        vk::DescriptorImageInfo
                        {
                            .imageView = frame_image_views[image_index],
                            .imageLayout = image_layout,
                        }
                    );
                }

                assert(binding_image_infos.size() == binding_image_indices.size());
                descriptor_set_image_infos.push_back(std::move(binding_image_infos));
            }

            assert(descriptor_set_image_infos.size() == descriptor_set_image_indices.size());
            image_infos.push_back(std::move(descriptor_set_image_infos));
        }

        assert(image_infos.size() == descriptor_sets_image_indices.size());
        return image_infos;
    }

    void update_frame_descriptor_sets(
        vk::Device const device,
        std::span<vk::DescriptorSet const> const frame_descriptor_sets,
        std::span<std::pmr::vector<std::pmr::vector<std::size_t>> const> descriptor_sets_image_indices,
        std::span<std::pmr::vector<std::pmr::vector<vk::ImageLayout>> const> descriptor_sets_image_layouts,
        std::span<vk::ImageView const> frame_image_views,
        std::span<std::pmr::vector<Frame_descriptor_set_binding> const> const descriptor_set_bindings,
        std::pmr::polymorphic_allocator<> const& temporaries_allocator
    )
    {
        assert(frame_descriptor_sets.size() == descriptor_set_bindings.size());

        std::pmr::vector<std::pmr::vector<std::pmr::vector<vk::DescriptorImageInfo>>> const descriptor_set_image_infos =
            create_frame_descriptor_set_image_infos(
                descriptor_sets_image_indices,
                descriptor_sets_image_layouts,
                frame_image_views,
                temporaries_allocator
            );

        std::pmr::vector<vk::WriteDescriptorSet> writes{ temporaries_allocator };

        for (std::size_t descriptor_set_index = 0; descriptor_set_index < frame_descriptor_sets.size(); ++descriptor_set_index)
        {
            vk::DescriptorSet const descriptor_set = frame_descriptor_sets[descriptor_set_index];

            std::span<Frame_descriptor_set_binding const> const bindings = descriptor_set_bindings[descriptor_set_index];
            std::span<std::pmr::vector<vk::DescriptorImageInfo> const> const binding_image_infos = descriptor_set_image_infos[descriptor_set_index];
            assert(bindings.size() == binding_image_infos.size());

            for (std::size_t binding_index = 0; binding_index < bindings.size(); ++binding_index)
            {
                Frame_descriptor_set_binding const binding = bindings[binding_index];
                std::span<vk::DescriptorImageInfo const> const image_infos = binding_image_infos[binding_index];

                writes.push_back(
                    vk::WriteDescriptorSet
                    {
                        .dstSet = descriptor_set,
                        .dstBinding = binding.binding,
                        .dstArrayElement = binding.first_array_element,
                        .descriptorCount = static_cast<std::uint32_t>(image_infos.size()),
                        .descriptorType = binding.descriptor_type,
                        .pImageInfo = image_infos.data(),
                    }
                );
            }
        }

        device.updateDescriptorSets(writes, {});
    }

    Frame_descriptor_sets_map create_frame_descriptor_sets_map(
        nlohmann::json const& frame_descriptor_sets_json,
        std::pmr::polymorphic_allocator<> const& output_allocator
    )
    {
        std::size_t const per_frame_descriptor_set_count =
            std::count_if(
                frame_descriptor_sets_json.begin(),
                frame_descriptor_sets_json.end(),
                [](nlohmann::json const& json) -> std::size_t { return json.at("type").get<std::string>() == "per_frame";}
        );

        std::size_t const shared_descriptor_set_count =
            std::count_if(
                frame_descriptor_sets_json.begin(),
                frame_descriptor_sets_json.end(),
                [](nlohmann::json const& json) -> std::size_t { return json.at("type").get<std::string>() == "shared";}
        );

        std::pmr::vector<std::size_t> per_frame_indices{ output_allocator };
        per_frame_indices.resize(per_frame_descriptor_set_count, std::numeric_limits<std::size_t>::max());

        std::pmr::vector<std::size_t> shared_indices{ output_allocator };
        shared_indices.resize(shared_descriptor_set_count, std::numeric_limits<std::size_t>::max());

        for (std::size_t to_index = 0; to_index < frame_descriptor_sets_json.size(); ++to_index)
        {
            nlohmann::json const& json = frame_descriptor_sets_json[to_index];

            std::string const& type = json.at("type").get<std::string>();
            std::size_t const from_index = json.at("index").get<std::size_t>();

            if (type == "per_frame")
            {
                per_frame_indices[from_index] = to_index;
            }
            else
            {
                assert(type == "shared");

                shared_indices[from_index] = to_index;
            }
        }

        auto const is_valid = [](std::size_t const index) -> bool { return index != std::numeric_limits<std::size_t>::max(); };
        assert(std::all_of(per_frame_indices.begin(), per_frame_indices.end(), is_valid));
        assert(std::all_of(shared_indices.begin(), shared_indices.end(), is_valid));

        return Frame_descriptor_sets_map
        {
            .per_frame_indices = std::move(per_frame_indices),
            .shared_indices = std::move(shared_indices),
        };
    }

    std::pmr::vector<vk::DescriptorSet> get_frame_descriptor_sets(
        std::span<vk::DescriptorSet const> const per_frame_descriptor_sets,
        std::span<vk::DescriptorSet const> const shared_descriptor_sets,
        Frame_descriptor_sets_map const& map,
        std::pmr::polymorphic_allocator<> const& output_allocator
    )
    {
        assert(per_frame_descriptor_sets.size() == map.per_frame_indices.size());
        assert(shared_descriptor_sets.size() == map.shared_indices.size());

        std::pmr::vector<vk::DescriptorSet> frame_descriptor_sets{ output_allocator };
        frame_descriptor_sets.resize(map.per_frame_indices.size() + map.shared_indices.size(), {});

        for (std::size_t from_index = 0; from_index < per_frame_descriptor_sets.size(); ++from_index)
        {
            std::size_t const to_index = map.per_frame_indices[from_index];
            frame_descriptor_sets[to_index] = per_frame_descriptor_sets[from_index];
        }

        for (std::size_t from_index = 0; from_index < shared_descriptor_sets.size(); ++from_index)
        {
            std::size_t const to_index = map.shared_indices[from_index];
            frame_descriptor_sets[to_index] = shared_descriptor_sets[from_index];
        }

        assert(map.per_frame_indices.size() + map.shared_indices.size());
        return frame_descriptor_sets;
    }

    namespace
    {
        std::pmr::vector<vk::AttachmentDescription> create_attachments(
            nlohmann::json const& attachments_json,
            std::pmr::polymorphic_allocator<> const& output_allocator
        ) noexcept
        {
            auto const create_attachment = [](nlohmann::json const& attachment_json) -> vk::AttachmentDescription
            {
                return
                {
                    .flags = attachment_json.at("flags").get<vk::AttachmentDescriptionFlags>(),
                    .format = attachment_json.at("format").get<vk::Format>(),
                    .samples = attachment_json.at("samples").get<vk::SampleCountFlagBits>(),
                    .loadOp = attachment_json.at("load_operation").get<vk::AttachmentLoadOp>(),
                    .storeOp = attachment_json.at("store_operation").get<vk::AttachmentStoreOp>(),
                    .stencilLoadOp = attachment_json.at("stencil_load_operation").get<vk::AttachmentLoadOp>(),
                    .stencilStoreOp = attachment_json.at("stencil_store_operation").get<vk::AttachmentStoreOp>(),
                    .initialLayout = attachment_json.at("initial_layout").get<vk::ImageLayout>(),
                    .finalLayout = attachment_json.at("final_layout").get<vk::ImageLayout>(),
                };
            };

            std::pmr::vector<vk::AttachmentDescription> attachments{ output_allocator };
            attachments.resize(attachments_json.size());

            std::transform(attachments_json.begin(), attachments_json.end(), attachments.begin(), create_attachment);

            return attachments;
        }

        vk::AttachmentReference create_attachment_reference(
            nlohmann::json const& attachment_reference_json
        ) noexcept
        {
            return
            {
                .attachment = attachment_reference_json.at("attachment").get<std::uint32_t>(),
                .layout = attachment_reference_json.at("layout").get<vk::ImageLayout>(),
            };
        };

        std::pmr::vector<vk::AttachmentReference> create_attachment_references(
            nlohmann::json const& attachment_references_json,
            std::pmr::polymorphic_allocator<> const& output_allocator
        ) noexcept
        {
            std::pmr::vector<vk::AttachmentReference> attachment_references{ output_allocator };
            attachment_references.resize(attachment_references_json.size());

            std::transform(attachment_references_json.begin(), attachment_references_json.end(), attachment_references.begin(), create_attachment_reference);

            return attachment_references;
        }

        std::size_t get_attachment_reference_count(
            nlohmann::json const& subpasses_json
        ) noexcept
        {
            std::size_t count = 0;

            for (nlohmann::json const& subpass_json : subpasses_json)
            {
                count += subpass_json.at("input_attachments").size()
                    + subpass_json.at("color_attachments").size()
                    + subpass_json.at("resolve_attachments").size()
                    + (!subpass_json.at("depth_stencil_attachment").empty() ? 1 : 0);

            }

            return count;
        }

        std::pmr::vector<vk::AttachmentReference> create_subpasses_attachment_references(
            nlohmann::json const& subpasses_json,
            std::pmr::polymorphic_allocator<> const& output_allocator,
            std::pmr::polymorphic_allocator<> const& temporaries_allocator
        ) noexcept
        {
            std::pmr::vector<vk::AttachmentReference> attachment_references{ output_allocator };
            attachment_references.reserve(get_attachment_reference_count(subpasses_json));

            for (nlohmann::json const& subpass_json : subpasses_json)
            {
                assert(subpass_json.at("resolve_attachments").empty() || (subpass_json.at("resolve_attachments").size() == subpass_json.at("color_attachments").size()));

                std::pmr::vector<vk::AttachmentReference> const input_attachments =
                    create_attachment_references(subpass_json.at("input_attachments"), temporaries_allocator);
                attachment_references.insert(attachment_references.end(), input_attachments.begin(), input_attachments.end());

                std::pmr::vector<vk::AttachmentReference> const color_attachments =
                    create_attachment_references(subpass_json.at("color_attachments"), temporaries_allocator);
                attachment_references.insert(attachment_references.end(), color_attachments.begin(), color_attachments.end());

                std::pmr::vector<vk::AttachmentReference> const resolve_attachments =
                    create_attachment_references(subpass_json.at("resolve_attachments"), temporaries_allocator);
                attachment_references.insert(attachment_references.end(), resolve_attachments.begin(), resolve_attachments.end());

                if (!subpass_json.at("depth_stencil_attachment").empty())
                {
                    vk::AttachmentReference const depth_stencil_attachment =
                        create_attachment_reference(subpass_json.at("depth_stencil_attachment"));

                    attachment_references.push_back(depth_stencil_attachment);
                }
            }

            return attachment_references;
        }

        std::pmr::vector<std::uint32_t> create_subpasses_preserve_attachments(
            nlohmann::json const& subpasses_json,
            std::pmr::polymorphic_allocator<> const& output_allocator
        ) noexcept
        {
            std::pmr::vector<std::uint32_t> preserve_attachments{ output_allocator };

            for (nlohmann::json const& subpass_json : subpasses_json)
            {
                for (nlohmann::json const& number_json : subpass_json.at("preserve_attachments"))
                {
                    preserve_attachments.push_back(number_json.get<std::uint32_t>());
                }
            }

            return preserve_attachments;
        }

        std::pmr::vector<vk::SubpassDescription> create_subpasses(
            nlohmann::json const& subpasses_json,
            std::span<vk::AttachmentReference const> const attachment_references,
            std::span<std::uint32_t const> const preserve_attachments,
            std::pmr::polymorphic_allocator<vk::SubpassDescription> const& output_allocator
        ) noexcept
        {
            std::pmr::vector<vk::SubpassDescription> subpasses{ output_allocator };
            subpasses.reserve(subpasses_json.size());

            vk::AttachmentReference const* current_attachment_reference = attachment_references.data();
            std::uint32_t const* current_preserve_attachment = preserve_attachments.data();

            for (nlohmann::json const& subpass_json : subpasses_json)
            {
                assert(subpass_json.at("resolve_attachments").empty() || (subpass_json.at("resolve_attachments").size() == subpass_json.at("color_attachments").size()));

                nlohmann::json const& input_attachments_json = subpass_json.at("input_attachments");
                nlohmann::json const& color_attachments_json = subpass_json.at("color_attachments");
                nlohmann::json const& resolve_attachments_json = subpass_json.at("resolve_attachments");
                nlohmann::json const& depth_stencil_attachment_json = subpass_json.at("depth_stencil_attachment");
                nlohmann::json const& preserve_attachment_json = subpass_json.at("preserve_attachments");

                vk::AttachmentReference const* const input_attachments_pointer =
                    !input_attachments_json.empty() ? current_attachment_reference : nullptr;
                current_attachment_reference += input_attachments_json.size();

                vk::AttachmentReference const* const color_attachments_pointer =
                    !color_attachments_json.empty() ? current_attachment_reference : nullptr;
                current_attachment_reference += color_attachments_json.size();

                vk::AttachmentReference const* const resolve_attachment_pointer =
                    !resolve_attachments_json.empty() ? current_attachment_reference : nullptr;
                current_attachment_reference += resolve_attachments_json.size();

                vk::AttachmentReference const* const depth_stencil_attachment_pointer =
                    !depth_stencil_attachment_json.empty() ? current_attachment_reference : nullptr;
                current_attachment_reference += (!depth_stencil_attachment_json.empty() ? 1 : 0);

                std::uint32_t const* const preserve_attachments_pointer =
                    !preserve_attachment_json.empty() ? current_preserve_attachment : nullptr;
                current_preserve_attachment += preserve_attachment_json.size();

                subpasses.push_back(
                    {
                        .flags = {},
                        .pipelineBindPoint = subpass_json.at("pipeline_bind_point").get<vk::PipelineBindPoint>(),
                        .inputAttachmentCount = static_cast<std::uint32_t>(input_attachments_json.size()),
                        .pInputAttachments = input_attachments_pointer,
                        .colorAttachmentCount = static_cast<std::uint32_t>(color_attachments_json.size()),
                        .pColorAttachments = color_attachments_pointer,
                        .pResolveAttachments = resolve_attachment_pointer,
                        .pDepthStencilAttachment = depth_stencil_attachment_pointer,
                        .preserveAttachmentCount = static_cast<std::uint32_t>(preserve_attachment_json.size()),
                        .pPreserveAttachments = preserve_attachments_pointer,
                    }
                );
            }

            return std::move(subpasses);
        }

        std::pmr::vector<vk::SubpassDependency> create_dependencies(
            nlohmann::json const& dependencies_json,
            std::pmr::polymorphic_allocator<vk::SubpassDependency> const& output_allocator
        ) noexcept
        {
            auto const create_dependency = [](nlohmann::json const& dependency_json) -> vk::SubpassDependency
            {
                auto const get_subpass_index = [](nlohmann::json const& subpass_index_json) -> std::uint32_t
                {
                    assert(subpass_index_json.is_number_unsigned() || subpass_index_json.get<std::string>() == "external");

                    if (subpass_index_json.is_number_unsigned())
                    {
                        return subpass_index_json.get<std::int32_t>();
                    }
                    else
                    {
                        return VK_SUBPASS_EXTERNAL;
                    }
                };

                return
                {
                    .srcSubpass = get_subpass_index(dependency_json.at("source_subpass")),
                    .dstSubpass = get_subpass_index(dependency_json.at("destination_subpass")),
                    .srcStageMask = dependency_json.at("source_stage_mask").get<vk::PipelineStageFlags>(),
                    .dstStageMask = dependency_json.at("destination_stage_mask").get<vk::PipelineStageFlags>(),
                    .srcAccessMask = dependency_json.at("source_access_mask").get<vk::AccessFlags>(),
                    .dstAccessMask = dependency_json.at("destination_access_mask").get<vk::AccessFlags>(),
                    .dependencyFlags = dependency_json.at("dependency_flags").get<vk::DependencyFlags>(),
                };
            };

            std::pmr::vector<vk::SubpassDependency> dependencies{ output_allocator };
            dependencies.resize(dependencies_json.size());

            std::transform(dependencies_json.begin(), dependencies_json.end(), dependencies.begin(), create_dependency);

            return dependencies;
        }
    }

    Render_pass_create_info_resources create_render_pass_create_info_resources(
        nlohmann::json const& render_pass_json,
        std::pmr::polymorphic_allocator<> const& output_allocator,
        std::pmr::polymorphic_allocator<> const& temporaries_allocator
    ) noexcept
    {
        std::pmr::vector<vk::AttachmentDescription> attachments = create_attachments(render_pass_json.at("attachments"), output_allocator);
        std::pmr::vector<vk::AttachmentReference> attachment_references = create_subpasses_attachment_references(render_pass_json.at("subpasses"), output_allocator, temporaries_allocator);
        std::pmr::vector<std::uint32_t> preserve_attachments = create_subpasses_preserve_attachments(render_pass_json.at("subpasses"), output_allocator);
        std::pmr::vector<vk::SubpassDescription> subpasses = create_subpasses(render_pass_json.at("subpasses"), attachment_references, preserve_attachments, output_allocator);
        std::pmr::vector<vk::SubpassDependency> dependencies = create_dependencies(render_pass_json.at("dependencies"), output_allocator);

        vk::RenderPassCreateInfo const create_info
        {
            .flags = {},
            .attachmentCount = static_cast<std::uint32_t>(attachments.size()),
            .pAttachments = attachments.data(),
            .subpassCount = static_cast<std::uint32_t>(subpasses.size()),
            .pSubpasses = subpasses.data(),
            .dependencyCount = static_cast<std::uint32_t>(dependencies.size()),
            .pDependencies = dependencies.data(),
        };

        return
        {
            .attachments = std::move(attachments),
            .attachment_references = std::move(attachment_references),
            .preserve_attachments = std::move(preserve_attachments),
            .subpasses = std::move(subpasses),
            .dependencies = std::move(dependencies),
            .create_info = create_info,
        };
    }

    std::pmr::vector<vk::RenderPass> create_render_passes(
        vk::Device const device,
        vk::AllocationCallbacks const* const allocation_callbacks,
        nlohmann::json const& render_passes_json,
        std::pmr::polymorphic_allocator<> const& output_allocator,
        std::pmr::polymorphic_allocator<> const& temporaries_allocator
    ) noexcept
    {
        std::pmr::vector<vk::RenderPass> render_passes{ output_allocator };
        render_passes.reserve(render_passes_json.size());

        for (nlohmann::json const& render_pass_json : render_passes_json)
        {
            Render_pass_create_info_resources const create_info_resources = create_render_pass_create_info_resources(
                render_pass_json,
                temporaries_allocator,
                temporaries_allocator
            );

            vk::RenderPass const render_pass = device.createRenderPass(
                create_info_resources.create_info,
                allocation_callbacks
            );

            render_passes.push_back(render_pass);
        }

        return render_passes;
    }

    namespace
    {
        std::pmr::vector<std::byte> read_bytes(
            std::filesystem::path const& file_path,
            std::pmr::polymorphic_allocator<> const& output_allocator
        ) noexcept
        {
            std::ifstream input_stream{ file_path, std::ios::in | std::ios::binary };
            assert(input_stream.good());

            input_stream.seekg(0, std::ios::end);
            auto const size_in_bytes = input_stream.tellg();

            std::pmr::vector<std::byte> buffer{ output_allocator };
            buffer.resize(size_in_bytes);

            input_stream.seekg(0, std::ios::beg);
            input_stream.read(reinterpret_cast<char*>(buffer.data()), buffer.size());

            return buffer;
        }

        template<typename Value_type>
        std::pmr::vector<Value_type> convert_bytes(
            std::span<std::byte const> const bytes,
            std::pmr::polymorphic_allocator<> const& output_allocator
        ) noexcept
        {
            assert(bytes.size_bytes() % sizeof(Value_type) == 0);

            std::pmr::vector<Value_type> values{ output_allocator };
            values.resize(bytes.size_bytes() / sizeof(Value_type));

            std::memcpy(values.data(), bytes.data(), bytes.size_bytes());

            return values;
        }
    }

    std::pmr::vector<vk::ShaderModule> create_shader_modules(
        vk::Device const device,
        vk::AllocationCallbacks const* const allocation_callbacks,
        nlohmann::json const& shader_modules_json,
        std::filesystem::path const& shaders_path,
        std::pmr::polymorphic_allocator<> const& output_allocator,
        std::pmr::polymorphic_allocator<> const& temporaries_allocator
    ) noexcept
    {
        std::pmr::vector<vk::ShaderModule> shader_modules{ output_allocator };
        shader_modules.reserve(shader_modules_json.size());

        for (nlohmann::json const& shader_module_json : shader_modules_json)
        {
            std::string const& shader_file = shader_module_json.at("file").get<std::string>();

            std::pmr::vector<std::uint32_t> const shader_code = convert_bytes<std::uint32_t>(read_bytes(shaders_path / shader_file, temporaries_allocator), temporaries_allocator);

            vk::ShaderModuleCreateInfo const create_info
            {
                .flags = {},
                .codeSize = shader_code.size() * sizeof(decltype(shader_code)::value_type),
                .pCode = shader_code.data(),
            };

            vk::ShaderModule const shader_module = device.createShaderModule(create_info, allocation_callbacks);

            shader_modules.push_back(shader_module);
        }

        return shader_modules;
    }

    std::pmr::vector<vk::Sampler> create_samplers(
        vk::Device const device,
        vk::AllocationCallbacks const* const allocation_callbacks,
        nlohmann::json const& samplers_json,
        std::pmr::polymorphic_allocator<> const& output_allocator
    ) noexcept
    {
        std::pmr::vector<vk::Sampler> samplers{ output_allocator };
        samplers.reserve(samplers_json.size());

        for (nlohmann::json const& sampler_json : samplers_json)
        {
            vk::SamplerCreateInfo const create_info
            {
                .flags = {},
                .magFilter = sampler_json.at("mag_filter").get<vk::Filter>(),
                .minFilter = sampler_json.at("min_filter").get<vk::Filter>(),
                .mipmapMode = sampler_json.at("mipmap_mode").get<vk::SamplerMipmapMode>(),
                .addressModeU = sampler_json.at("address_mode_u").get<vk::SamplerAddressMode>(),
                .addressModeV = sampler_json.at("address_mode_v").get<vk::SamplerAddressMode>(),
                .addressModeW = sampler_json.at("address_mode_w").get<vk::SamplerAddressMode>(),
                .mipLodBias = sampler_json.at("mip_lod_bias").get<float>(),
                .anisotropyEnable = sampler_json.at("anisotropy_enable").get<vk::Bool32>(),
                .maxAnisotropy = sampler_json.at("max_anisotropy").get<float>(),
                .compareEnable = sampler_json.at("compare_enable").get<vk::Bool32>(),
                .compareOp = sampler_json.at("compare_operation").get<vk::CompareOp>(),
                .minLod = sampler_json.at("min_lod").get<float>(),
                .maxLod = sampler_json.at("max_lod").get<float>(),
                .borderColor = sampler_json.at("border_color").get<vk::BorderColor>(),
                .unnormalizedCoordinates = sampler_json.at("unnormalized_coordinates").get<vk::Bool32>(),
            };

            vk::Sampler const sampler = device.createSampler(create_info, allocation_callbacks);

            samplers.push_back(sampler);
        }

        return samplers;
    }

    namespace
    {
        std::pmr::vector<vk::Sampler> arrange_immutable_samplers(
            nlohmann::json const& descriptor_set_layouts_json,
            std::span<vk::Sampler const> const samplers,
            std::pmr::polymorphic_allocator<> const& output_allocator
        ) noexcept
        {
            std::pmr::vector<vk::Sampler> immutable_samplers_per_descriptor_set_binding{ output_allocator };

            for (nlohmann::json const& descriptor_set_layout_json : descriptor_set_layouts_json)
            {
                for (nlohmann::json const& binding_json : descriptor_set_layout_json.at("bindings"))
                {
                    for (nlohmann::json const& immutable_sampler_json : binding_json.at("immutable_samplers"))
                    {
                        std::size_t const sampler_index = immutable_sampler_json.get<std::size_t>();

                        immutable_samplers_per_descriptor_set_binding.push_back(
                            samplers[sampler_index]
                        );
                    }
                }
            }

            return immutable_samplers_per_descriptor_set_binding;
        }

        std::pmr::vector<vk::DescriptorSetLayoutBinding> create_descriptor_set_layouts_bindings(
            nlohmann::json const& descriptor_set_layouts_json,
            std::span<vk::Sampler const> const immutable_samplers_per_descriptor_set_binding,
            std::pmr::polymorphic_allocator<> const& output_allocator
        ) noexcept
        {
            std::pmr::vector<vk::DescriptorSetLayoutBinding> bindings{ output_allocator };

            std::size_t start_sampler_index = 0;

            for (nlohmann::json const& descriptor_set_layout_json : descriptor_set_layouts_json)
            {
                for (nlohmann::json const& binding_json : descriptor_set_layout_json.at("bindings"))
                {
                    nlohmann::json const& immutable_samplers_json = binding_json.at("immutable_samplers");

                    vk::DescriptorSetLayoutBinding const binding
                    {
                        .binding = binding_json.at("binding").get<std::uint32_t>(),
                        .descriptorType = binding_json.at("descriptor_type").get<vk::DescriptorType>(),
                        .descriptorCount = binding_json.at("descriptor_count").get<std::uint32_t>(),
                        .stageFlags = binding_json.at("stage_flags_property").get<vk::ShaderStageFlags>(),
                        .pImmutableSamplers =
                            !immutable_samplers_json.empty() ?
                            immutable_samplers_per_descriptor_set_binding.data() + start_sampler_index :
                            nullptr
                    };

                    bindings.push_back(binding);

                    start_sampler_index += immutable_samplers_json.size();
                }
            }

            return bindings;
        }
    }

    std::pmr::vector<vk::DescriptorSetLayout> create_descriptor_set_layouts(
        vk::Device const device,
        vk::AllocationCallbacks const* const allocation_callbacks,
        std::span<vk::Sampler const> const samplers,
        nlohmann::json const& descriptor_set_layouts_json,
        std::pmr::polymorphic_allocator<> const& output_allocator,
        std::pmr::polymorphic_allocator<> const& temporaries_allocator
    ) noexcept
    {
        std::pmr::vector<vk::Sampler> const immutable_samplers_per_descriptor_set_binding =
            arrange_immutable_samplers(descriptor_set_layouts_json, samplers, temporaries_allocator);

        std::pmr::vector<vk::DescriptorSetLayoutBinding> const bindings =
            create_descriptor_set_layouts_bindings(descriptor_set_layouts_json, immutable_samplers_per_descriptor_set_binding, temporaries_allocator);

        std::pmr::vector<vk::DescriptorSetLayout> descriptor_set_layouts{ output_allocator };
        descriptor_set_layouts.reserve(descriptor_set_layouts_json.size());

        std::uint32_t start_binding_index = 0;

        for (nlohmann::json const& descriptor_set_layout_json : descriptor_set_layouts_json)
        {
            std::uint32_t const num_bindings = static_cast<std::uint32_t>(descriptor_set_layout_json.at("bindings").size());

            vk::DescriptorSetLayoutCreateInfo const create_info
            {
                .flags = {},
                .bindingCount = num_bindings,
                .pBindings = bindings.data() + start_binding_index,
            };

            vk::DescriptorSetLayout const descriptor_set_layout =
                device.createDescriptorSetLayout(create_info, allocation_callbacks);

            descriptor_set_layouts.push_back(descriptor_set_layout);
            start_binding_index += num_bindings;
        }

        return descriptor_set_layouts;
    }

    namespace
    {
        std::pmr::vector<vk::DescriptorSetLayout> arrange_descriptor_set_layouts(
            nlohmann::json const& pipeline_layouts_json,
            std::span<vk::DescriptorSetLayout const> const descriptor_set_layouts,
            std::pmr::polymorphic_allocator<> const& output_allocator
        ) noexcept
        {
            std::pmr::vector<vk::DescriptorSetLayout> descriptor_set_layouts_per_pipeline_layout{ output_allocator };

            for (nlohmann::json const& pipeline_layout_json : pipeline_layouts_json)
            {
                for (nlohmann::json const& descriptor_set_layout_json : pipeline_layout_json.at("descriptor_set_layouts"))
                {
                    std::size_t const layout_index = descriptor_set_layout_json.get<std::size_t>();

                    descriptor_set_layouts_per_pipeline_layout.push_back(
                        descriptor_set_layouts[layout_index]
                    );
                }
            }

            return descriptor_set_layouts_per_pipeline_layout;
        }

        std::pmr::vector<vk::PushConstantRange> create_push_constant_ranges(
            nlohmann::json const& pipeline_layouts_json,
            std::pmr::polymorphic_allocator<> const& output_allocator
        ) noexcept
        {
            auto const create_push_constant_range = [](nlohmann::json const& push_constant_range_json) -> vk::PushConstantRange
            {
                return
                {
                    .stageFlags = push_constant_range_json.at("stage_flags").get<vk::ShaderStageFlags>(),
                    .offset = push_constant_range_json.at("offset").get<std::uint32_t>(),
                    .size = push_constant_range_json.at("size").get<std::uint32_t>(),
                };
            };

            std::pmr::vector<vk::PushConstantRange> push_constant_ranges{ output_allocator };

            for (nlohmann::json const& pipeline_layout_json : pipeline_layouts_json)
            {
                for (nlohmann::json const& push_constant_range_json : pipeline_layout_json.at("push_constant_ranges"))
                {
                    push_constant_ranges.push_back(
                        create_push_constant_range(push_constant_range_json)
                    );
                }
            }

            return push_constant_ranges;
        }
    }

    std::pmr::vector<vk::PipelineLayout> create_pipeline_layouts(
        vk::Device const device,
        vk::AllocationCallbacks const* const allocation_callbacks,
        std::span<vk::DescriptorSetLayout const> const descriptor_set_layouts,
        nlohmann::json const& pipeline_layouts_json,
        std::pmr::polymorphic_allocator<> const& output_allocator,
        std::pmr::polymorphic_allocator<> const& temporaries_allocator
    ) noexcept
    {
        std::pmr::vector<vk::DescriptorSetLayout> const descriptor_set_layouts_per_pipeline_layout =
            arrange_descriptor_set_layouts(pipeline_layouts_json, descriptor_set_layouts, temporaries_allocator);

        std::pmr::vector<vk::PushConstantRange> const push_constant_ranges =
            create_push_constant_ranges(pipeline_layouts_json, temporaries_allocator);

        std::pmr::vector<vk::PipelineLayout> pipeline_layouts{ output_allocator };
        pipeline_layouts.reserve(pipeline_layouts_json.size());

        std::uint32_t start_descriptor_set_layout_index = 0;
        std::uint32_t start_push_constant_range_index = 0;

        for (nlohmann::json const& pipeline_layout_json : pipeline_layouts_json)
        {
            std::uint32_t const num_descriptor_set_layouts = static_cast<std::uint32_t>(pipeline_layout_json.at("descriptor_set_layouts").size());
            std::uint32_t const num_push_constant_ranges = static_cast<std::uint32_t>(pipeline_layout_json.at("push_constant_ranges").size());

            vk::PipelineLayoutCreateInfo const create_info
            {
                .flags = {},
                .setLayoutCount = num_descriptor_set_layouts,
                .pSetLayouts = descriptor_set_layouts_per_pipeline_layout.data() + start_descriptor_set_layout_index,
                .pushConstantRangeCount = num_push_constant_ranges,
                .pPushConstantRanges = push_constant_ranges.data() + start_push_constant_range_index,
            };

            vk::PipelineLayout const pipeline_layout =
                device.createPipelineLayout(create_info, allocation_callbacks);

            pipeline_layouts.push_back(pipeline_layout);
            start_descriptor_set_layout_index += num_descriptor_set_layouts;
            start_push_constant_range_index += num_push_constant_ranges;
        }

        return pipeline_layouts;
    }


    namespace
    {
        std::pmr::vector<std::pmr::string> create_pipeline_shader_stage_names(
            nlohmann::json const& pipeline_states_json,
            std::pmr::polymorphic_allocator<> const& output_allocator
        ) noexcept
        {
            std::pmr::vector<std::pmr::string> stage_names{ output_allocator };

            for (nlohmann::json const& pipeline_state_json : pipeline_states_json)
            {
                for (nlohmann::json const& stage_json : pipeline_state_json.at("stages"))
                {
                    std::string const& name = stage_json.at("entry_point").get<std::string>();

                    stage_names.push_back(
                        std::pmr::string{ name, output_allocator }
                    );
                }
            }

            return stage_names;
        }

        vk::PipelineShaderStageCreateInfo create_pipeline_shader_stage_create_info(
            nlohmann::json const& json,
            std::span<vk::ShaderModule const> const shader_modules,
            char const* const name
        ) noexcept
        {
            return
            {
                .flags = {},
                .stage = json.at("stage").get<vk::ShaderStageFlagBits>(),
                .module = shader_modules[json.at("shader").get<std::size_t>()],
                .pName = name,
                .pSpecializationInfo = nullptr,
            };
        }

        std::pmr::vector<vk::PipelineShaderStageCreateInfo> create_pipeline_shader_stage_create_infos(
            nlohmann::json const& pipeline_states_json,
            std::span<vk::ShaderModule const> const shader_modules,
            std::span<std::pmr::string const> const stage_names,
            std::pmr::polymorphic_allocator<> const& output_allocator
        ) noexcept
        {
            std::pmr::vector<vk::PipelineShaderStageCreateInfo> create_infos{ output_allocator };

            std::size_t stage_name_index = 0;

            for (nlohmann::json const& pipeline_state_json : pipeline_states_json)
            {
                for (nlohmann::json const& stage_json : pipeline_state_json.at("stages"))
                {
                    create_infos.push_back(
                        create_pipeline_shader_stage_create_info(stage_json, shader_modules, stage_names[stage_name_index].c_str())
                    );

                    ++stage_name_index;
                }
            }

            return create_infos;
        }

        vk::VertexInputBindingDescription create_vertex_input_binding_description(
            nlohmann::json const& json
        ) noexcept
        {
            return
            {
                .binding = json.at("binding").get<std::uint32_t>(),
                .stride = json.at("stride").get<std::uint32_t>(),
                .inputRate = json.at("input_rate").get<vk::VertexInputRate>(),
            };
        }

        std::pmr::vector<vk::VertexInputBindingDescription> create_vertex_input_binding_descriptions(
            nlohmann::json const& pipeline_states_json,
            std::pmr::polymorphic_allocator<> const& output_allocator
        ) noexcept
        {
            std::pmr::vector<vk::VertexInputBindingDescription> descriptions{ output_allocator };

            for (nlohmann::json const& pipeline_state_json : pipeline_states_json)
            {
                for (nlohmann::json const& binding_json : pipeline_state_json.at("vertex_input_state").at("bindings"))
                {
                    descriptions.push_back(
                        create_vertex_input_binding_description(binding_json)
                    );
                }
            }

            return descriptions;
        }

        vk::VertexInputAttributeDescription create_vertex_input_attribute_description(
            nlohmann::json const& json
        ) noexcept
        {
            return
            {
                .location = json.at("location").get<std::uint32_t>(),
                .binding = json.at("binding").get<std::uint32_t>(),
                .format = json.at("format").get<vk::Format>(),
                .offset = json.at("offset").get<std::uint32_t>(),
            };
        }

        std::pmr::vector<vk::VertexInputAttributeDescription> create_vertex_input_attribute_descriptions(
            nlohmann::json const& pipeline_states_json,
            std::pmr::polymorphic_allocator<> const& output_allocator
        ) noexcept
        {
            std::pmr::vector<vk::VertexInputAttributeDescription> descriptions{ output_allocator };

            for (nlohmann::json const& pipeline_state_json : pipeline_states_json)
            {
                for (nlohmann::json const& binding_json : pipeline_state_json.at("vertex_input_state").at("attributes"))
                {
                    descriptions.push_back(
                        create_vertex_input_attribute_description(binding_json)
                    );
                }
            }

            return descriptions;
        }


        vk::PipelineVertexInputStateCreateInfo create_pipeline_vertex_input_state_create_info(
            nlohmann::json const& json,
            std::span<vk::VertexInputBindingDescription const> const bindings,
            std::span<vk::VertexInputAttributeDescription const> const attributes
        ) noexcept
        {
            return
            {
                .flags = {},
                .vertexBindingDescriptionCount = static_cast<std::uint32_t>(bindings.size()),
                .pVertexBindingDescriptions = bindings.data(),
                .vertexAttributeDescriptionCount = static_cast<std::uint32_t>(attributes.size()),
                .pVertexAttributeDescriptions = attributes.data(),
            };
        }

        std::pmr::vector<vk::PipelineVertexInputStateCreateInfo> create_pipeline_vertex_input_state_create_infos(
            nlohmann::json const& pipeline_states_json,
            std::span<vk::VertexInputBindingDescription const> const bindings,
            std::span<vk::VertexInputAttributeDescription const> const attributes,
            std::pmr::polymorphic_allocator<> const& output_allocator
        ) noexcept
        {
            std::pmr::vector<vk::PipelineVertexInputStateCreateInfo> create_infos{ output_allocator };
            create_infos.reserve(pipeline_states_json.size());

            std::size_t start_binding_index = 0;
            std::size_t start_attribute_index = 0;

            for (nlohmann::json const& pipeline_state_json : pipeline_states_json)
            {
                nlohmann::json const& vertex_input_state_json = pipeline_state_json.at("vertex_input_state");

                std::size_t const binding_count = vertex_input_state_json.at("bindings").size();
                std::size_t const attribute_count = vertex_input_state_json.at("attributes").size();

                create_infos.push_back(
                    create_pipeline_vertex_input_state_create_info(
                        vertex_input_state_json,
                        { bindings.data() + start_binding_index, binding_count },
                        { attributes.data() + start_attribute_index, start_attribute_index }
                    )
                );

                start_binding_index += binding_count;
                start_attribute_index += attribute_count;
            }

            return create_infos;
        }


        vk::PipelineInputAssemblyStateCreateInfo create_pipeline_input_assembly_state_create_info(
            nlohmann::json const& json
        ) noexcept
        {
            return
            {
                .flags = {},
                .topology = json.at("topology").get<vk::PrimitiveTopology>(),
                .primitiveRestartEnable = json.at("primitive_restart_enable").get<vk::Bool32>(),
            };
        }

        std::pmr::vector<vk::PipelineInputAssemblyStateCreateInfo> create_pipeline_input_assembly_state_create_infos(
            nlohmann::json const& pipeline_states_json,
            std::pmr::polymorphic_allocator<> const& output_allocator
        ) noexcept
        {
            std::pmr::vector<vk::PipelineInputAssemblyStateCreateInfo> create_infos{ output_allocator };

            for (nlohmann::json const& pipeline_state_json : pipeline_states_json)
            {
                create_infos.push_back(
                    create_pipeline_input_assembly_state_create_info(pipeline_state_json.at("input_assembly_state"))
                );
            }

            return create_infos;
        }


        vk::PipelineTessellationStateCreateInfo create_pipeline_tessellation_state_create_info(
            nlohmann::json const& json
        ) noexcept
        {
            return {};
        }

        std::pmr::vector<std::optional<vk::PipelineTessellationStateCreateInfo>> create_pipeline_tessellation_state_create_infos(
            nlohmann::json const& pipeline_states_json,
            std::pmr::polymorphic_allocator<> const& output_allocator
        ) noexcept
        {
            std::pmr::vector<std::optional<vk::PipelineTessellationStateCreateInfo>> create_infos{ output_allocator };
            create_infos.reserve(pipeline_states_json.size());

            for (nlohmann::json const& pipeline_state_json : pipeline_states_json)
            {
                create_infos.push_back(
                    {}
                );
            }

            return create_infos;
        }

        vk::Viewport create_viewport(
            nlohmann::json const& json
        ) noexcept
        {
            return
            {
                .x = json.at("x").get<float>(),
                .y = json.at("y").get<float>(),
                .width = json.at("width").get<float>(),
                .height = json.at("height").get<float>(),
                .minDepth = json.at("minimum").get<float>(),
                .maxDepth = json.at("maximum").get<float>(),
            };
        }

        std::pmr::vector<vk::Viewport> create_viewports(
            nlohmann::json const& pipeline_states_json,
            std::pmr::polymorphic_allocator<> const& output_allocator
        ) noexcept
        {
            std::pmr::vector<vk::Viewport> viewports{ output_allocator };

            for (nlohmann::json const& pipeline_state_json : pipeline_states_json)
            {
                for (nlohmann::json const& viewport_json : pipeline_state_json.at("viewport_state").at("viewports"))
                {
                    viewports.push_back(
                        create_viewport(viewport_json)
                    );
                }
            }

            return viewports;
        }

        vk::Offset2D create_offset_2d(
            nlohmann::json const& json
        ) noexcept
        {
            return
            {
                .x = json.at("x").get<std::int32_t>(),
                .y = json.at("y").get<std::int32_t>(),
            };
        }

        vk::Extent2D create_extent_2d(
            nlohmann::json const& json
        ) noexcept
        {
            return
            {
                .width = json.at("width").get<std::uint32_t>(),
                .height = json.at("height").get<std::uint32_t>(),
            };
        }

        vk::Rect2D create_rect_2d(
            nlohmann::json const& json
        ) noexcept
        {
            return
            {
                .offset = create_offset_2d(json.at("offset")),
                .extent = create_extent_2d(json.at("extent")),
            };
        }

        std::pmr::vector<vk::Rect2D> create_scissors(
            nlohmann::json const& pipeline_states_json,
            std::pmr::polymorphic_allocator<> const& output_allocator
        ) noexcept
        {
            std::pmr::vector<vk::Rect2D> scissors{ output_allocator };

            for (nlohmann::json const& pipeline_state_json : pipeline_states_json)
            {
                for (nlohmann::json const& scissor_json : pipeline_state_json.at("viewport_state").at("scissors"))
                {
                    scissors.push_back(
                        create_rect_2d(scissor_json)
                    );
                }
            }

            return scissors;
        }


        vk::PipelineViewportStateCreateInfo create_pipeline_viewport_state_create_info(
            nlohmann::json const& json,
            std::span<vk::Viewport const> const viewports,
            std::span<vk::Rect2D const> const scissors
        ) noexcept
        {
            assert(json.at("viewports").size() == viewports.size());
            assert(json.at("scissors").size() == scissors.size());
            assert((json.at("viewport_count").get<std::uint32_t>() == json.at("viewports").size()) || json.at("viewports").is_null());
            assert((json.at("scissor_count").get<std::uint32_t>() == json.at("scissors").size()) || json.at("scissors").is_null());

            return
            {
                .flags = {},
                .viewportCount = json.at("viewport_count").get<std::uint32_t>(),
                .pViewports = !viewports.empty() ? viewports.data() : nullptr,
                .scissorCount = json.at("scissor_count").get<std::uint32_t>(),
                .pScissors = !scissors.empty() ? scissors.data() : nullptr,
            };
        }

        std::pmr::vector<vk::PipelineViewportStateCreateInfo> create_pipeline_viewport_state_create_infos(
            nlohmann::json const& pipeline_states_json,
            std::span<vk::Viewport const> const viewports,
            std::span<vk::Rect2D const> const scissors,
            std::pmr::polymorphic_allocator<> const& output_allocator
        ) noexcept
        {
            std::pmr::vector<vk::PipelineViewportStateCreateInfo> create_infos{ output_allocator };
            create_infos.reserve(pipeline_states_json.size());

            std::size_t start_viewport_index = 0;
            std::size_t start_scissor_index = 0;

            for (nlohmann::json const& pipeline_state_json : pipeline_states_json)
            {
                nlohmann::json const& viewport_state_json = pipeline_state_json.at("viewport_state");

                std::size_t const viewport_count = viewport_state_json.at("viewports").size();
                std::size_t const scissor_count = viewport_state_json.at("scissors").size();

                create_infos.push_back(
                    create_pipeline_viewport_state_create_info(
                        viewport_state_json,
                        { viewports.data() + start_viewport_index, viewport_count },
                        { scissors.data() + start_scissor_index, scissor_count }
                    )
                );

                start_viewport_index += viewport_count;
                start_scissor_index += scissor_count;
            }

            return create_infos;
        }


        vk::PipelineRasterizationStateCreateInfo create_pipeline_rasterization_state_create_info(
            nlohmann::json const& json
        ) noexcept
        {
            return
            {
                .flags = {},
                .depthClampEnable = json.at("depth_clamp_enable").get<vk::Bool32>(),
                .rasterizerDiscardEnable = json.at("rasterizer_discard_enable").get<vk::Bool32>(),
                .polygonMode = json.at("polygon_mode").get<vk::PolygonMode>(),
                .cullMode = json.at("cull_mode").get<vk::CullModeFlags>(),
                .frontFace = json.at("front_face").get<vk::FrontFace>(),
                .depthBiasEnable = json.at("depth_bias_enable").get<vk::Bool32>(),
                .depthBiasConstantFactor = json.at("depth_bias_constant_factor").get<float>(),
                .depthBiasClamp = json.at("depth_bias_clamp").get<float>(),
                .depthBiasSlopeFactor = json.at("depth_bias_slope_factor").get<float>(),
                .lineWidth = json.at("line_width_factor").get<float>(),
            };
        }

        std::pmr::vector<vk::PipelineRasterizationStateCreateInfo> create_pipeline_rasterization_state_create_infos(
            nlohmann::json const& pipeline_states_json,
            std::pmr::polymorphic_allocator<> const& output_allocator
        ) noexcept
        {
            std::pmr::vector<vk::PipelineRasterizationStateCreateInfo> create_infos{ output_allocator };

            for (nlohmann::json const& pipeline_state_json : pipeline_states_json)
            {
                create_infos.push_back(
                    create_pipeline_rasterization_state_create_info(pipeline_state_json.at("rasterization_state"))
                );
            }

            return create_infos;
        }


        vk::PipelineMultisampleStateCreateInfo create_pipeline_multisample_state_create_info(
            nlohmann::json const& json
        ) noexcept
        {
            static std::uint32_t constexpr sample_mask = 0xFFFFFFFF;

            return
            {
                .flags = {},
                .rasterizationSamples = vk::SampleCountFlagBits::e1,
                .sampleShadingEnable = VK_FALSE,
                .minSampleShading = {},
                .pSampleMask = &sample_mask,
                .alphaToCoverageEnable = VK_FALSE,
                .alphaToOneEnable = VK_FALSE,
            };
        }

        std::pmr::vector<vk::PipelineMultisampleStateCreateInfo> create_pipeline_multisample_state_create_infos(
            nlohmann::json const& pipeline_states_json,
            std::pmr::polymorphic_allocator<> const& output_allocator
        ) noexcept
        {
            std::pmr::vector<vk::PipelineMultisampleStateCreateInfo> create_infos{ output_allocator };
            create_infos.reserve(pipeline_states_json.size());

            for (nlohmann::json const& pipeline_state_json : pipeline_states_json)
            {
                create_infos.push_back(
                    create_pipeline_multisample_state_create_info({})
                );
            }

            return create_infos;
        }

        vk::StencilOpState create_stencil_operation_state(
            nlohmann::json const& json
        ) noexcept
        {
            return
            {
                .failOp = json.at("fail_operation").get<vk::StencilOp>(),
                .passOp = json.at("pass_operation").get<vk::StencilOp>(),
                .depthFailOp = json.at("depth_fail_operation").get<vk::StencilOp>(),
                .compareOp = json.at("compare_operation").get<vk::CompareOp>(),
                .compareMask = json.at("compare_mask").get<std::uint32_t>(),
                .writeMask = json.at("write_mask").get<std::uint32_t>(),
                .reference = json.at("reference").get<std::uint32_t>(),
            };
        }

        vk::PipelineDepthStencilStateCreateInfo create_pipeline_depth_stencil_state_create_info(
            nlohmann::json const& json
        ) noexcept
        {
            return
            {
                .flags = {},
                .depthTestEnable = json.at("depth_test_enable").get<vk::Bool32>(),
                .depthWriteEnable = json.at("depth_write_enable").get<vk::Bool32>(),
                .depthCompareOp = json.at("compare_operation").get<vk::CompareOp>(),
                .depthBoundsTestEnable = json.at("depth_bounds_test_enable").get<vk::Bool32>(),
                .stencilTestEnable = json.at("stencil_test_enable").get<vk::Bool32>(),
                .front = create_stencil_operation_state(json.at("front_stencil_state")),
                .back = create_stencil_operation_state(json.at("back_stencil_state")),
                .minDepthBounds = json.at("min_depth_bounds").get<float>(),
                .maxDepthBounds = json.at("max_depth_bounds").get<float>(),
            };
        }

        std::pmr::vector<vk::PipelineDepthStencilStateCreateInfo> create_pipeline_depth_stencil_state_create_infos(
            nlohmann::json const& pipeline_states_json,
            std::pmr::polymorphic_allocator<> const& output_allocator
        ) noexcept
        {
            std::pmr::vector<vk::PipelineDepthStencilStateCreateInfo> create_infos{ output_allocator };

            for (nlohmann::json const& pipeline_state_json : pipeline_states_json)
            {
                create_infos.push_back(
                    create_pipeline_depth_stencil_state_create_info(pipeline_state_json.at("depth_stencil_state"))
                );
            }

            return create_infos;
        }


        vk::PipelineColorBlendAttachmentState create_pipeline_color_blend_attachment_state(
            nlohmann::json const& json
        ) noexcept
        {
            return
            {
                .blendEnable = json.at("blend_enable").get<vk::Bool32>(),
                .srcColorBlendFactor = json.at("source_color_blend_factor").get<vk::BlendFactor>(),
                .dstColorBlendFactor = json.at("destination_color_blend_factor").get<vk::BlendFactor>(),
                .colorBlendOp = json.at("color_blend_operation").get<vk::BlendOp>(),
                .srcAlphaBlendFactor = json.at("source_alpha_blend_factor").get<vk::BlendFactor>(),
                .dstAlphaBlendFactor = json.at("destination_alpha_blend_factor").get<vk::BlendFactor>(),
                .alphaBlendOp = json.at("alpha_blend_operation").get<vk::BlendOp>(),
                .colorWriteMask = json.at("color_write_mask").get<vk::ColorComponentFlags>(),
            };
        }

        std::pmr::vector<vk::PipelineColorBlendAttachmentState> create_pipeline_color_blend_attachment_states(
            nlohmann::json const& pipeline_states_json,
            std::pmr::polymorphic_allocator<> const& output_allocator
        ) noexcept
        {
            std::pmr::vector<vk::PipelineColorBlendAttachmentState> attachments{ output_allocator };

            for (nlohmann::json const& pipeline_state_json : pipeline_states_json)
            {
                for (nlohmann::json const& attachment_json : pipeline_state_json.at("color_blend_state").at("attachments"))
                {
                    attachments.push_back(
                        create_pipeline_color_blend_attachment_state(
                            attachment_json
                        )
                    );
                }
            }

            return attachments;
        }

        vk::PipelineColorBlendStateCreateInfo create_pipeline_color_blend_state_create_info(
            nlohmann::json const& json,
            std::span<vk::PipelineColorBlendAttachmentState const> const attachments
        ) noexcept
        {
            assert(attachments.size() == json.at("attachments").size());

            nlohmann::json const& blend_constants_json = json.at("blend_constants");
            assert(blend_constants_json.size() == 4);

            std::array<float, 4> const blend_constants =
            {
                static_cast<float>(blend_constants_json[0]),
                static_cast<float>(blend_constants_json[1]),
                static_cast<float>(blend_constants_json[2]),
                static_cast<float>(blend_constants_json[3]),
            };

            return vk::PipelineColorBlendStateCreateInfo
            {
                .flags = {},
                .logicOpEnable = json.at("logic_operation_enable").get<vk::Bool32>(),
                .logicOp = json.at("logic_operation").get<vk::LogicOp>(),
                .attachmentCount = static_cast<std::uint32_t>(attachments.size()),
                .pAttachments = attachments.data(),
                .blendConstants = blend_constants,
            };
        }

        std::pmr::vector<vk::PipelineColorBlendStateCreateInfo> create_pipeline_color_blend_state_create_infos(
            nlohmann::json const& pipeline_states_json,
            std::span<vk::PipelineColorBlendAttachmentState const> const attachments,
            std::pmr::polymorphic_allocator<> const& output_allocator
        ) noexcept
        {
            std::pmr::vector<vk::PipelineColorBlendStateCreateInfo> create_infos{ output_allocator };
            create_infos.reserve(pipeline_states_json.size());

            std::size_t start_attachment_index = 0;

            for (nlohmann::json const& pipeline_state_json : pipeline_states_json)
            {
                nlohmann::json const& color_blend_state_json = pipeline_state_json.at("color_blend_state");

                std::size_t const attachment_count = color_blend_state_json.at("attachments").size();

                create_infos.push_back(
                    create_pipeline_color_blend_state_create_info(
                        color_blend_state_json,
                        { attachments.data() + start_attachment_index, attachment_count }
                    )
                );
            }

            return create_infos;
        }


        std::pmr::vector<vk::DynamicState> create_dynamic_states(
            nlohmann::json const& pipeline_states_json,
            std::pmr::polymorphic_allocator<> const& output_allocator
        ) noexcept
        {
            std::pmr::vector<vk::DynamicState> dynamic_states{ output_allocator };

            for (nlohmann::json const& pipeline_state_json : pipeline_states_json)
            {
                for (nlohmann::json const& dynamic_state_json : pipeline_state_json.at("dynamic_state").at("dynamic_states"))
                {
                    dynamic_states.push_back(
                        dynamic_state_json.get<vk::DynamicState>()
                    );
                }
            }

            return dynamic_states;
        }

        vk::PipelineDynamicStateCreateInfo create_pipeline_dynamic_state_create_info(
            nlohmann::json const& json,
            std::span<vk::DynamicState const> const dynamic_states
        ) noexcept
        {
            assert(dynamic_states.size() == json.at("dynamic_states").size());

            return
            {
                .flags = {},
                .dynamicStateCount = static_cast<std::uint32_t>(dynamic_states.size()),
                .pDynamicStates = dynamic_states.data(),
            };
        }

        std::pmr::vector<vk::PipelineDynamicStateCreateInfo> create_pipeline_dynamic_state_create_infos(
            nlohmann::json const& pipeline_states_json,
            std::span<vk::DynamicState const> const dynamic_states,
            std::pmr::polymorphic_allocator<> const& output_allocator
        ) noexcept
        {
            std::pmr::vector<vk::PipelineDynamicStateCreateInfo> create_infos{ output_allocator };
            create_infos.reserve(pipeline_states_json.size());

            std::size_t start_dynamic_state_index = 0;

            for (nlohmann::json const& pipeline_state_json : pipeline_states_json)
            {
                nlohmann::json const& pipeline_dynamic_state_json = pipeline_state_json.at("dynamic_state");

                std::size_t const dynamic_state_count = pipeline_dynamic_state_json.at("dynamic_states").size();

                create_infos.push_back(
                    create_pipeline_dynamic_state_create_info(
                        pipeline_dynamic_state_json,
                        { dynamic_states.data() + start_dynamic_state_index, dynamic_state_count }
                    )
                );

                start_dynamic_state_index += dynamic_state_count;
            }

            return create_infos;
        }

    }

    std::pmr::vector<vk::Pipeline> create_graphics_pipeline_states(
        vk::Device const device,
        vk::PipelineCache const pipeline_cache,
        vk::AllocationCallbacks const* const allocation_callbacks,
        std::span<vk::ShaderModule const> const shader_modules,
        std::span<vk::PipelineLayout const> const pipeline_layouts,
        std::span<vk::RenderPass const> const render_passes,
        nlohmann::json const& pipeline_states_json,
        std::pmr::polymorphic_allocator<> const& output_allocator,
        std::pmr::polymorphic_allocator<> const& temporaries_allocator
    )
    {
        std::pmr::vector<std::pmr::string> const shader_stage_names =
            create_pipeline_shader_stage_names(
                pipeline_states_json,
                temporaries_allocator
            );

        std::pmr::vector<vk::PipelineShaderStageCreateInfo> const shader_stages =
            create_pipeline_shader_stage_create_infos(
                pipeline_states_json,
                shader_modules,
                shader_stage_names,
                temporaries_allocator
            );

        std::pmr::vector<vk::VertexInputBindingDescription> const vertex_input_binding_descriptions =
            create_vertex_input_binding_descriptions(
                pipeline_states_json,
                temporaries_allocator
            );

        std::pmr::vector<vk::VertexInputAttributeDescription> const vertex_input_attribute_descriptions =
            create_vertex_input_attribute_descriptions(
                pipeline_states_json,
                temporaries_allocator
            );

        std::pmr::vector<vk::PipelineVertexInputStateCreateInfo> const vertex_input_states =
            create_pipeline_vertex_input_state_create_infos(
                pipeline_states_json,
                vertex_input_binding_descriptions,
                vertex_input_attribute_descriptions,
                temporaries_allocator
            );

        std::pmr::vector<vk::PipelineInputAssemblyStateCreateInfo> const input_assembly_states =
            create_pipeline_input_assembly_state_create_infos(
                pipeline_states_json,
                temporaries_allocator
            );

        std::pmr::vector<std::optional<vk::PipelineTessellationStateCreateInfo>> const tessellation_states =
            create_pipeline_tessellation_state_create_infos(
                pipeline_states_json,
                temporaries_allocator
            );

        std::pmr::vector<vk::Viewport> const viewports = create_viewports(pipeline_states_json, temporaries_allocator);

        std::pmr::vector<vk::Rect2D> const scissors = create_scissors(pipeline_states_json, temporaries_allocator);

        std::pmr::vector<vk::PipelineViewportStateCreateInfo> const viewport_states =
            create_pipeline_viewport_state_create_infos(
                pipeline_states_json,
                viewports,
                scissors,
                temporaries_allocator
            );

        std::pmr::vector<vk::PipelineRasterizationStateCreateInfo> const rasterization_states =
            create_pipeline_rasterization_state_create_infos(
                pipeline_states_json,
                temporaries_allocator
            );

        std::pmr::vector<vk::PipelineMultisampleStateCreateInfo> const multisample_states =
            create_pipeline_multisample_state_create_infos(
                pipeline_states_json,
                temporaries_allocator
            );

        std::pmr::vector<vk::PipelineDepthStencilStateCreateInfo> const depth_stencil_states =
            create_pipeline_depth_stencil_state_create_infos(
                pipeline_states_json,
                temporaries_allocator
            );

        std::pmr::vector<vk::PipelineColorBlendAttachmentState> const color_blend_attachment_states =
            create_pipeline_color_blend_attachment_states(
                pipeline_states_json,
                temporaries_allocator
            );

        std::pmr::vector<vk::PipelineColorBlendStateCreateInfo> const color_blend_states =
            create_pipeline_color_blend_state_create_infos(
                pipeline_states_json,
                color_blend_attachment_states,
                temporaries_allocator
            );

        std::pmr::vector<vk::DynamicState> const dynamic_states = create_dynamic_states(pipeline_states_json, temporaries_allocator);

        std::pmr::vector<vk::PipelineDynamicStateCreateInfo> const pipeline_dynamic_states =
            create_pipeline_dynamic_state_create_infos(
                pipeline_states_json,
                dynamic_states,
                temporaries_allocator
            );


        std::pmr::vector<vk::GraphicsPipelineCreateInfo> create_infos{ temporaries_allocator };
        create_infos.reserve(pipeline_states_json.size());

        {
            std::size_t pipeline_state_index = 0;
            std::size_t start_stage_index = 0;

            for (nlohmann::json const& pipeline_state_json : pipeline_states_json)
            {
                std::size_t const stage_count = pipeline_state_json.at("stages").size();

                vk::GraphicsPipelineCreateInfo const create_info
                {
                    .flags = {},
                    .stageCount = static_cast<std::uint32_t>(stage_count),
                    .pStages = shader_stages.data() + start_stage_index,
                    .pVertexInputState = vertex_input_states.data() + pipeline_state_index,
                    .pInputAssemblyState = input_assembly_states.data() + pipeline_state_index,
                    .pTessellationState = tessellation_states[pipeline_state_index].has_value() ? &tessellation_states[pipeline_state_index].value() : nullptr,
                    .pViewportState = viewport_states.data() + pipeline_state_index,
                    .pRasterizationState = rasterization_states.data() + pipeline_state_index,
                    .pMultisampleState = multisample_states.data() + pipeline_state_index,
                    .pDepthStencilState = depth_stencil_states.data() + pipeline_state_index,
                    .pColorBlendState = color_blend_states.data() + pipeline_state_index,
                    .pDynamicState = pipeline_dynamic_states.data() + pipeline_state_index,
                    .layout = pipeline_layouts[pipeline_state_json.at("pipeline_layout").get<std::size_t>()],
                    .renderPass = render_passes[pipeline_state_json.at("render_pass").get<std::size_t>()],
                    .subpass = pipeline_state_json.at("subpass").get<std::uint32_t>(),
                    .basePipelineHandle = {},
                    .basePipelineIndex = {},
                };

                create_infos.push_back(create_info);

                pipeline_state_index += 1;
                start_stage_index += stage_count;
            }
        }

        if (!create_infos.empty())
        {
            std::pmr::polymorphic_allocator<vk::Pipeline> pipelines_vector_allocator{ output_allocator };
            std::pmr::vector<vk::Pipeline> pipelines =
                device.createGraphicsPipelines(pipeline_cache, create_infos, allocation_callbacks, pipelines_vector_allocator).value;

            return pipelines;
        }
        else
        {
            return {};
        }
    }

    std::pmr::vector<vk::RayTracingShaderGroupCreateInfoKHR> create_ray_tracing_shader_group_create_infos(
        nlohmann::json const& pipeline_states_json,
        std::pmr::polymorphic_allocator<> const& output_allocator
    )
    {
        std::pmr::vector<vk::RayTracingShaderGroupCreateInfoKHR> create_infos{ output_allocator };

        for (nlohmann::json const& pipeline_state_json : pipeline_states_json)
        {
            for (nlohmann::json const& group_json : pipeline_state_json.at("groups"))
            {
                vk::RayTracingShaderGroupCreateInfoKHR const create_info
                {
                    .type = group_json.at("type").get<vk::RayTracingShaderGroupTypeKHR>(),
                    .generalShader = group_json.contains("general_shader") ? group_json.at("general_shader").get<std::uint32_t>() : VK_SHADER_UNUSED_KHR,
                    .closestHitShader = group_json.contains("closest_hit_shader") ? group_json.at("closest_hit_shader").get<std::uint32_t>() : VK_SHADER_UNUSED_KHR,
                    .anyHitShader = group_json.contains("any_hit_shader") ? group_json.at("any_hit_shader").get<std::uint32_t>() : VK_SHADER_UNUSED_KHR,
                    .intersectionShader = group_json.contains("intersection_shader") ? group_json.at("intersection_shader").get<std::uint32_t>() : VK_SHADER_UNUSED_KHR,
                    .pShaderGroupCaptureReplayHandle = nullptr,
                };

                create_infos.push_back(create_info);
            }
        }

        return create_infos;
    }

    std::pmr::vector<vk::Pipeline> gather_pipeline_libraries(
        nlohmann::json const& pipeline_state_json,
        std::span<vk::Pipeline const> const pipelines,
        std::pmr::polymorphic_allocator<> const& output_allocator
    )
    {
        if (!pipeline_state_json.contains("library_info"))
        {
            return std::pmr::vector<vk::Pipeline>{ output_allocator };
        }

        auto const get_pipeline_library = [=](nlohmann::json const& library_index_json) -> vk::Pipeline
        {
            std::size_t const index = library_index_json.get<std::size_t>();

            return pipelines[index];
        };

        nlohmann::json const& library_info_json = pipeline_state_json.at("library_info");

        std::pmr::vector<vk::Pipeline> pipeline_libraries{ output_allocator };
        pipeline_libraries.resize(library_info_json.size());


        std::transform(
            library_info_json.begin(),
            library_info_json.end(),
            pipeline_libraries.begin(),
            get_pipeline_library
        );

        return pipeline_libraries;
    }

    std::pmr::vector<std::optional<vk::RayTracingPipelineInterfaceCreateInfoKHR>> create_ray_tracing_pipeline_interface_create_infos(
        nlohmann::json const& pipeline_states_json,
        std::pmr::polymorphic_allocator<> const& output_allocator
    )
    {
        auto const get_create_info = [](nlohmann::json const& pipeline_state_json) -> std::optional<vk::RayTracingPipelineInterfaceCreateInfoKHR>
        {
            if (pipeline_state_json.contains("library_interface"))
            {
                nlohmann::json const& library_interface_json = pipeline_state_json.at("library_interface");

                vk::RayTracingPipelineInterfaceCreateInfoKHR const create_info
                {
                    .maxPipelineRayPayloadSize = library_interface_json.at("max_pipeline_ray_payload_size").get<std::uint32_t>(),
                    .maxPipelineRayHitAttributeSize = library_interface_json.at("max_pipeline_ray_hit_attribute_size").get<std::uint32_t>(),
                };

                return create_info;
            }
            else
            {
                return std::nullopt;
            }
        };

        std::pmr::vector<std::optional<vk::RayTracingPipelineInterfaceCreateInfoKHR>> create_infos{ output_allocator };
        create_infos.resize(pipeline_states_json.size());


        std::transform(
            pipeline_states_json.begin(),
            pipeline_states_json.end(),
            create_infos.begin(),
            get_create_info
        );

        return create_infos;
    }

    std::pmr::vector<vk::Pipeline> create_ray_tracing_pipeline_states(
        vk::Device const device,
        vk::PipelineCache const pipeline_cache,
        vk::AllocationCallbacks const* const allocation_callbacks,
        std::span<vk::ShaderModule const> const shader_modules,
        std::span<vk::PipelineLayout const> const pipeline_layouts,
        nlohmann::json const& pipeline_states_json,
        std::pmr::polymorphic_allocator<> const& output_allocator,
        std::pmr::polymorphic_allocator<> const& temporaries_allocator
    )
    {
        std::pmr::vector<std::pmr::string> const shader_stage_names =
            create_pipeline_shader_stage_names(
                pipeline_states_json,
                temporaries_allocator
            );

        std::pmr::vector<vk::PipelineShaderStageCreateInfo> const shader_stages =
            create_pipeline_shader_stage_create_infos(
                pipeline_states_json,
                shader_modules,
                shader_stage_names,
                temporaries_allocator
            );

        std::pmr::vector<vk::RayTracingShaderGroupCreateInfoKHR> const groups =
            create_ray_tracing_shader_group_create_infos(
                pipeline_states_json,
                temporaries_allocator
            );

        std::pmr::vector<std::optional<vk::RayTracingPipelineInterfaceCreateInfoKHR>> const library_interfaces =
            create_ray_tracing_pipeline_interface_create_infos(
                pipeline_states_json,
                temporaries_allocator
            );

        std::pmr::vector<vk::DynamicState> const dynamic_states = create_dynamic_states(pipeline_states_json, temporaries_allocator);

        std::pmr::vector<vk::PipelineDynamicStateCreateInfo> const pipeline_dynamic_states =
            create_pipeline_dynamic_state_create_infos(
                pipeline_states_json,
                dynamic_states,
                temporaries_allocator
            );

        std::pmr::vector<vk::Pipeline> pipelines{ output_allocator };
        pipelines.resize(pipeline_states_json.size());

        {
            std::size_t start_stage_index = 0;
            std::size_t start_group_index = 0;

            for (std::size_t index = 0; index < pipeline_states_json.size(); ++index)
            {
                nlohmann::json const& pipeline_state_json = pipeline_states_json[index];

                std::uint32_t const shader_stage_count = static_cast<std::uint32_t>(pipeline_state_json.at("stages").size());
                std::uint32_t const group_count = static_cast<std::uint32_t>(pipeline_state_json.at("groups").size());

                std::pmr::vector<vk::Pipeline> const pipeline_libraries =
                    gather_pipeline_libraries(
                        pipeline_state_json,
                        pipelines,
                        temporaries_allocator
                    );

                vk::PipelineLibraryCreateInfoKHR const library_info
                {
                    .libraryCount = static_cast<std::uint32_t>(pipeline_libraries.size()),
                    .pLibraries = !pipeline_libraries.empty() ? pipeline_libraries.data() : nullptr
                };

                std::array<vk::RayTracingPipelineCreateInfoKHR, 1> const create_info
                {
                    vk::RayTracingPipelineCreateInfoKHR
                    {
                        .flags = pipeline_state_json.at("flags").get<vk::PipelineCreateFlagBits>(),
                        .stageCount = shader_stage_count,
                        .pStages = shader_stages.data() + start_stage_index,
                        .groupCount = group_count,
                        .pGroups = groups.data() + start_group_index,
                        .maxPipelineRayRecursionDepth = pipeline_state_json.at("max_pipeline_ray_recursion_depth").get<std::uint32_t>(),
                        .pLibraryInfo = &library_info,
                        .pLibraryInterface = library_interfaces[index].has_value() ? &library_interfaces[index].value() : nullptr,
                        .pDynamicState = &pipeline_dynamic_states[index],
                        .layout = pipeline_layouts[pipeline_state_json.at("layout").get<std::size_t>()],
                        .basePipelineHandle = {},
                        .basePipelineIndex = {},
                    }
                };

                std::pmr::polymorphic_allocator<vk::Pipeline> allocator{ temporaries_allocator };
                std::pmr::vector<vk::Pipeline> const pipeline = device.createRayTracingPipelinesKHR(
                    {},
                    pipeline_cache,
                    create_info,
                    allocation_callbacks,
                    allocator
                ).value;

                pipelines[index] = pipeline[0];

                start_stage_index += shader_stage_count;
                start_group_index += group_count;
            }
        }

        return pipelines;
    }

    std::pmr::vector<vk::Pipeline> gather_pipeline_states(
        std::span<vk::Pipeline const> const compute_pipeline_states,
        std::span<vk::Pipeline const> const graphics_pipeline_states,
        std::span<vk::Pipeline const> const raytracing_pipeline_states,
        nlohmann::json const& pipeline_states_json,
        std::pmr::polymorphic_allocator<> const& output_allocator
    )
    {
        auto const get_pipeline_state = [=](nlohmann::json const& pipeline_state_json) -> vk::Pipeline
        {
            std::string const& type = pipeline_state_json.at("type").get<std::string>();
            std::size_t const index = pipeline_state_json.at("index").get<std::size_t>();

            if (type == "compute")
            {
                return compute_pipeline_states[index];
            }
            else if (type == "graphics")
            {
                return graphics_pipeline_states[index];
            }
            else if (type == "ray_tracing")
            {
                return raytracing_pipeline_states[index];
            }
            else
            {
                throw std::runtime_error{ "Pipeline type is not recognized!" };
            }
        };

        std::pmr::vector<vk::Pipeline> pipeline_states{ output_allocator };
        pipeline_states.resize(pipeline_states_json.size());

        std::transform(
            pipeline_states_json.begin(),
            pipeline_states_json.end(),
            pipeline_states.begin(),
            get_pipeline_state
        );

        return pipeline_states;
    }

    template <typename T, typename S>
    T align(T const value, S const alignment) noexcept
    {
        T const remainder = (value % alignment);
        T const aligned_value = (remainder == 0) ? value : (value + (alignment - remainder));
        assert((aligned_value % alignment) == 0);
        return aligned_value;
    }

    std::pmr::vector<Maia::Renderer::Vulkan::Buffer_view> create_shader_binding_table_buffer_views(
        vk::PhysicalDeviceRayTracingPipelinePropertiesKHR const& physical_device_properties,
        Maia::Renderer::Vulkan::Buffer_resources& shader_binding_tables_buffer_resources,
        nlohmann::json const& shader_binding_tables_json,
        std::pmr::polymorphic_allocator<> const& output_allocator
    )
    {
        assert((shader_binding_tables_buffer_resources.usage() & vk::BufferUsageFlagBits::eShaderBindingTableKHR));

        using Buffer_view = Maia::Renderer::Vulkan::Buffer_view;

        std::uint32_t const handle_size_aligned = align(physical_device_properties.shaderGroupHandleSize, physical_device_properties.shaderGroupHandleAlignment);
        std::uint32_t const alignment = physical_device_properties.shaderGroupBaseAlignment;

        std::pmr::vector<Buffer_view> buffer_views{ output_allocator };
        buffer_views.reserve(shader_binding_tables_json.size());

        for (nlohmann::json const& shader_binding_table_json : shader_binding_tables_json)
        {
            std::uint32_t const group_count = shader_binding_table_json.at("group_count").get<std::uint32_t>();

            vk::DeviceSize const required_size = group_count * handle_size_aligned;
            Buffer_view const buffer_view = shader_binding_tables_buffer_resources.allocate_buffer(required_size, alignment, vk::MemoryPropertyFlagBits::eDeviceLocal);

            buffer_views.push_back(buffer_view);
        }

        return buffer_views;
    }

    void upload_shader_binding_tables_data(
        vk::Device const device,
        vk::Queue const upload_queue,
        vk::CommandPool const command_pool,
        std::span<vk::Pipeline const> const pipeline_states,
        vk::PhysicalDeviceRayTracingPipelinePropertiesKHR const& physical_device_properties,
        std::span<Maia::Renderer::Vulkan::Buffer_view const> const shader_binding_table_buffer_views,
        Maia::Renderer::Vulkan::Upload_buffer const* const upload_buffer,
        vk::AllocationCallbacks const* allocation_callbacks,
        nlohmann::json const& shader_binding_tables_json,
        std::pmr::polymorphic_allocator<> const& temporaries_allocator
    )
    {
        assert(shader_binding_tables_json.size() == shader_binding_table_buffer_views.size());

        std::uint32_t const handle_size_aligned = align(physical_device_properties.shaderGroupHandleSize, physical_device_properties.shaderGroupHandleAlignment);
        std::uint32_t const alignment = physical_device_properties.shaderGroupBaseAlignment;

        for (std::size_t index = 0; index < shader_binding_tables_json.size(); ++index)
        {
            nlohmann::json const& shader_binding_table_json = shader_binding_tables_json[index];
            Maia::Renderer::Vulkan::Buffer_view const& buffer_view = shader_binding_table_buffer_views[index];

            vk::Pipeline const pipeline = pipeline_states[shader_binding_table_json.at("pipeline_state_index").get<std::size_t>()];
            std::uint32_t const first_group = shader_binding_table_json.at("first_group").get<std::uint32_t>();
            std::uint32_t const group_count = shader_binding_table_json.at("group_count").get<std::uint32_t>();

            std::pmr::vector<std::byte> shader_group_handles{ temporaries_allocator };
            shader_group_handles.resize(physical_device_properties.shaderGroupHandleSize * group_count);

            {
                vk::Result const result = device.getRayTracingShaderGroupHandlesKHR(pipeline, first_group, group_count, shader_group_handles.size(), shader_group_handles.data());
                if (result != vk::Result::eSuccess)
                {
                    throw std::runtime_error{ "device.getRayTracingShaderGroupHandlesKHR() failed!" };
                }
            }

            std::pmr::vector<std::byte> aligned_shader_group_handles{ temporaries_allocator };
            aligned_shader_group_handles.resize(handle_size_aligned * group_count, std::byte{});

            for (std::size_t group_index = 0; group_index < group_count; ++group_index)
            {
                std::size_t const source = group_index * physical_device_properties.shaderGroupHandleSize;
                std::size_t const destination = group_index * handle_size_aligned;
                std::memcpy(aligned_shader_group_handles.data() + destination, shader_group_handles.data() + source, physical_device_properties.shaderGroupHandleSize);
            }

            Maia::Renderer::Vulkan::upload_data(
                device,
                upload_queue,
                command_pool,
                buffer_view,
                aligned_shader_group_handles,
                upload_buffer,
                allocation_callbacks
            );
        }
    }

    vk::StridedDeviceAddressRegionKHR create_shader_binding_table(
        vk::Device const device,
        vk::PhysicalDeviceRayTracingPipelinePropertiesKHR const& physical_device_properties,
        Maia::Renderer::Vulkan::Buffer_view const& shader_binding_table_buffer_view
    )
    {
        std::uint32_t const handle_size_aligned = align(physical_device_properties.shaderGroupHandleSize, physical_device_properties.shaderGroupHandleAlignment);

        vk::BufferDeviceAddressInfo const buffer_device_address_info =
        {
            .buffer = shader_binding_table_buffer_view.buffer,
        };
        vk::DeviceAddress const shader_binding_table_device_address = device.getBufferAddress(buffer_device_address_info) + shader_binding_table_buffer_view.offset;

        vk::StridedDeviceAddressRegionKHR const shader_binding_table
        {
            .deviceAddress = shader_binding_table_device_address,
            .stride = handle_size_aligned,
            .size = shader_binding_table_buffer_view.size,
        };

        return shader_binding_table;
    }

    std::pmr::vector<vk::StridedDeviceAddressRegionKHR> create_shader_binding_tables(
        vk::Device const device,
        vk::PhysicalDeviceRayTracingPipelinePropertiesKHR const& physical_device_properties,
        std::span<Maia::Renderer::Vulkan::Buffer_view const> const shader_binding_table_buffer_views,
        std::pmr::polymorphic_allocator<> const& output_allocator
    )
    {
        using Buffer_view = Maia::Renderer::Vulkan::Buffer_view;

        std::pmr::vector<vk::StridedDeviceAddressRegionKHR> shader_binding_tables{ output_allocator };
        shader_binding_tables.reserve(shader_binding_table_buffer_views.size());

        for (std::size_t index = 0; index < shader_binding_table_buffer_views.size(); ++index)
        {
            Maia::Renderer::Vulkan::Buffer_view const& shader_binding_table_buffer_view = shader_binding_table_buffer_views[index];

            vk::StridedDeviceAddressRegionKHR const shader_binding_table =
                create_shader_binding_table(
                    device,
                    physical_device_properties,
                    shader_binding_table_buffer_view
                );

            shader_binding_tables.push_back(shader_binding_table);
        }

        return shader_binding_tables;
    }

    Pipeline_resources::Pipeline_resources(
        vk::PhysicalDevice const physical_device,
        vk::PhysicalDeviceType const physical_device_type,
        vk::Device const device,
        vk::Queue const upload_queue,
        vk::CommandPool const command_pool,
        vk::PipelineCache const pipeline_cache,
        vk::PhysicalDeviceRayTracingPipelinePropertiesKHR const& physical_device_ray_tracing_properties,
        Maia::Renderer::Vulkan::Buffer_resources& shader_binding_tables_buffer_resources,
        Maia::Renderer::Vulkan::Upload_buffer const* const upload_buffer,
        vk::AllocationCallbacks const* const allocation_callbacks,
        nlohmann::json const& pipeline_json,
        std::filesystem::path const& pipeline_json_parent_path,
        std::pmr::polymorphic_allocator<> const& output_allocator,
        std::pmr::polymorphic_allocator<> const& temporaries_allocator
    ) noexcept :
        device{ device },
        allocation_callbacks{ allocation_callbacks },
        buffer_resources{ create_buffer_resources(pipeline_json.contains("buffers") ? pipeline_json.at("buffers") : nlohmann::json{}, physical_device, device, physical_device_type, allocation_callbacks, output_allocator) },
        image_resources{ physical_device, device, physical_device_type, {}, 64 * 1024 * 1024, {}, {}, allocation_callbacks, output_allocator }
    {
        this->render_passes =
            pipeline_json.contains("render_passes") ?
            Maia::Renderer::Vulkan::create_render_passes(
                device,
                allocation_callbacks,
                pipeline_json.at("render_passes"),
                output_allocator,
                temporaries_allocator
            ) :
            std::pmr::vector<vk::RenderPass>{ output_allocator };

        this->shader_modules =
            pipeline_json.contains("shader_modules") ?
            Maia::Renderer::Vulkan::create_shader_modules(
                device,
                allocation_callbacks,
                pipeline_json.at("shader_modules"),
                pipeline_json_parent_path,
                output_allocator,
                temporaries_allocator
            ) :
            std::pmr::vector<vk::ShaderModule>{ output_allocator };

        this->samplers =
            pipeline_json.contains("samplers") ?
            Maia::Renderer::Vulkan::create_samplers(
                device,
                allocation_callbacks,
                pipeline_json.at("samplers"),
                output_allocator
            ) :
            std::pmr::vector<vk::Sampler>{ output_allocator };

        this->descriptor_set_layouts =
            pipeline_json.contains("descriptor_set_layouts") ?
            Maia::Renderer::Vulkan::create_descriptor_set_layouts(
                device,
                allocation_callbacks,
                samplers,
                pipeline_json.at("descriptor_set_layouts"),
                output_allocator,
                temporaries_allocator
            ) :
            std::pmr::vector<vk::DescriptorSetLayout>{ output_allocator };

        this->pipeline_layouts =
            pipeline_json.contains("pipeline_layouts") ?
            Maia::Renderer::Vulkan::create_pipeline_layouts(
                device,
                allocation_callbacks,
                descriptor_set_layouts,
                pipeline_json.at("pipeline_layouts"),
                output_allocator,
                temporaries_allocator
            ) :
            std::pmr::vector<vk::PipelineLayout>{ output_allocator };

        if (pipeline_json.contains("pipeline_states"))
        {
            nlohmann::json const& pipeline_states_json = pipeline_json.at("pipeline_states");

            std::pmr::vector<vk::Pipeline> const compute_pipelines; // TODO

            std::pmr::vector<vk::Pipeline> const graphics_pipelines =
                pipeline_states_json.contains("graphics_pipeline_states") ?
                create_graphics_pipeline_states(
                    device,
                    pipeline_cache,
                    allocation_callbacks,
                    shader_modules,
                    pipeline_layouts,
                    render_passes,
                    pipeline_states_json.at("graphics_pipeline_states"),
                    temporaries_allocator,
                    temporaries_allocator
                ) :
                std::pmr::vector<vk::Pipeline>{ temporaries_allocator };

            std::pmr::vector<vk::Pipeline> const ray_tracing_pipelines =
                pipeline_states_json.contains("ray_tracing_pipeline_states") ?
                create_ray_tracing_pipeline_states(
                    device,
                    pipeline_cache,
                    allocation_callbacks,
                    shader_modules,
                    pipeline_layouts,
                    pipeline_states_json.at("ray_tracing_pipeline_states"),
                    temporaries_allocator,
                    temporaries_allocator
                ) :
                std::pmr::vector<vk::Pipeline>{ temporaries_allocator };

            this->pipeline_states = gather_pipeline_states(
                compute_pipelines,
                graphics_pipelines,
                ray_tracing_pipelines,
                pipeline_states_json.at("pipeline_states"),
                output_allocator
            );
        }

        if (pipeline_json.contains("shader_binding_tables"))
        {
            nlohmann::json const& shader_binding_tables_json = pipeline_json.at("shader_binding_tables");

            this->shader_binding_table_buffer_views = create_shader_binding_table_buffer_views(
                physical_device_ray_tracing_properties,
                shader_binding_tables_buffer_resources,
                shader_binding_tables_json,
                output_allocator
            );

            upload_shader_binding_tables_data(
                device,
                upload_queue,
                command_pool,
                this->pipeline_states,
                physical_device_ray_tracing_properties,
                this->shader_binding_table_buffer_views,
                upload_buffer,
                allocation_callbacks,
                shader_binding_tables_json,
                temporaries_allocator
            );

            this->shader_binding_tables = create_shader_binding_tables(
                device,
                physical_device_ray_tracing_properties,
                this->shader_binding_table_buffer_views,
                output_allocator
            );
        }

        if (pipeline_json.contains("buffers"))
        {
            this->buffer_memory_views = create_buffers(pipeline_json.at("buffers"), this->buffer_resources, output_allocator);

            if (pipeline_json.contains("buffer_views"))
            {
                this->buffer_views = create_buffer_views(pipeline_json.at("buffer_views"), device, this->buffer_memory_views, allocation_callbacks, output_allocator);
            }
        }

        if (pipeline_json.contains("images"))
        {
            this->image_memory_views = create_images(pipeline_json.at("images"), this->image_resources, output_allocator);

            if (pipeline_json.contains("image_views"))
            {
                this->image_views = create_image_views(pipeline_json.at("image_views"), device, this->image_memory_views, allocation_callbacks, output_allocator);
            }
        }

        if (pipeline_json.contains("descriptor_sets"))
        {
            nlohmann::json const& descriptor_sets_json = pipeline_json.at("descriptor_sets");

            this->descriptor_pool = create_descriptor_pool(
                descriptor_sets_json,
                pipeline_json.at("descriptor_set_layouts"),
                device,
                1,
                allocation_callbacks,
                temporaries_allocator
            );

            this->descriptor_sets = create_descriptor_sets(
                descriptor_sets_json,
                device,
                this->descriptor_pool,
                this->descriptor_set_layouts,
                output_allocator,
                temporaries_allocator
            );

            update_descriptor_sets(
                descriptor_sets_json,
                device,
                this->descriptor_sets,
                this->buffer_memory_views,
                this->buffer_views,
                this->image_views,
                {},
                {},
                temporaries_allocator
            );
        }
    }

    Pipeline_resources::~Pipeline_resources() noexcept
    {
        this->device.freeDescriptorSets(this->descriptor_pool, descriptor_sets);

        this->device.destroy(descriptor_pool, this->allocation_callbacks);

        for (vk::ImageView const image_view : image_views)
        {
            this->device.destroy(image_view, this->allocation_callbacks);
        }

        for (vk::BufferView const buffer_view : buffer_views)
        {
            this->device.destroy(buffer_view, this->allocation_callbacks);
        }

        for (vk::Pipeline const pipeline_state : pipeline_states)
        {
            this->device.destroy(pipeline_state, this->allocation_callbacks);
        }

        for (vk::PipelineLayout const pipeline_layout : pipeline_layouts)
        {
            this->device.destroy(pipeline_layout, this->allocation_callbacks);
        }

        for (vk::DescriptorSetLayout const descriptor_set_layout : descriptor_set_layouts)
        {
            this->device.destroy(descriptor_set_layout, this->allocation_callbacks);
        }

        for (vk::Sampler const sampler : samplers)
        {
            this->device.destroy(sampler, this->allocation_callbacks);
        }

        for (vk::ShaderModule const shader_module : shader_modules)
        {
            this->device.destroy(shader_module, this->allocation_callbacks);
        }

        for (vk::RenderPass const render_passe : render_passes)
        {
            this->device.destroy(render_passe, this->allocation_callbacks);
        }
    }

    namespace
    {
        enum class Command_type : std::uint8_t
        {
            Begin_render_pass,
            Bind_descriptor_sets,
            Bind_pipeline,
            Clear_color_image,
            Draw,
            End_render_pass,
            Pipeline_barrier,
            Trace_rays,
            Set_screen_viewport_and_scissors
        };

        template <typename... Types>
        std::pmr::vector<std::byte> write_command_data(
            std::pmr::polymorphic_allocator<> const& output_allocator,
            Types&&... command_data
        )
        {
            std::size_t constexpr data_size = (0 + ... + sizeof(std::remove_cvref_t<Types>));

            std::pmr::vector<std::byte> data{ output_allocator };
            data.resize(data_size);

            std::size_t offset = 0;

            auto const write_single_command = [&data, &offset] <typename Type> (Type const& command) -> void
            {
                std::memcpy(data.data() + offset, &command, sizeof(Type));
                offset += sizeof(Type);
            };

            (write_single_command(command_data), ...);

            return data;
        }

        struct Bind_descriptor_sets
        {
            vk::PipelineBindPoint pipeline_bind_point = {};
            vk::PipelineLayout pipeline_layout = {};
            std::uint32_t first_set = {};
            std::uint32_t descriptor_set_count = {};
            std::uint32_t dynamic_offset_count = {};

            enum class Bind_type : std::uint8_t
            {
                Frame_resource,
                Pipeline_resource,
            };

            struct Frame_resource
            {
                std::uint32_t descriptor_set_index = {};
            };

            struct Pipeline_resource
            {
                vk::DescriptorSet descriptor_set = {};
            };
        };

        std::pmr::vector<std::byte> create_bind_descriptor_sets_data(
            nlohmann::json const& json,
            std::span<vk::DescriptorSet const> const pipeline_descriptor_sets,
            std::span<vk::PipelineLayout const> const pipeline_layouts,
            std::pmr::polymorphic_allocator<> const& output_allocator,
            std::pmr::polymorphic_allocator<> const& temporaries_allocator
        ) noexcept
        {
            Bind_descriptor_sets const bind_descriptor_sets
            {
                .pipeline_bind_point = json.at("pipeline_bind_point").get<vk::PipelineBindPoint>(),
                .pipeline_layout = pipeline_layouts[json.at("pipeline_layout").get<std::size_t>()],
                .first_set = json.at("first_set").get<std::uint32_t>(),
                .descriptor_set_count = static_cast<std::uint32_t>(json.at("descriptor_sets").size()),
                .dynamic_offset_count = json.contains("dynamic_offsets") ? static_cast<std::uint32_t>(json.at("dynamic_offsets").size()) : 0,
            };

            std::pmr::vector<std::byte> data = write_command_data(temporaries_allocator, Command_type::Bind_descriptor_sets, bind_descriptor_sets);
            data.reserve(data.size() + bind_descriptor_sets.descriptor_set_count * (sizeof(Bind_descriptor_sets::Bind_type) + sizeof(vk::DescriptorSet)) + bind_descriptor_sets.dynamic_offset_count * sizeof(std::uint32_t));

            for (nlohmann::json const& descriptor_set_json : json.at("descriptor_sets"))
            {
                std::string const& type = descriptor_set_json.at("type").get<std::string>();

                if (type == "frame_resource")
                {
                    Bind_descriptor_sets::Frame_resource const frame_resource
                    {
                        .descriptor_set_index = descriptor_set_json.at("index").get<std::uint32_t>(),
                    };

                    std::pmr::vector<std::byte> const descriptor_set_data = write_command_data(temporaries_allocator, Bind_descriptor_sets::Bind_type::Frame_resource, frame_resource);
                    data.insert(data.end(), descriptor_set_data.begin(), descriptor_set_data.end());
                }
                else if (type == "pipeline_resource")
                {
                    Bind_descriptor_sets::Pipeline_resource const pipeline_resource
                    {
                        .descriptor_set = pipeline_descriptor_sets[descriptor_set_json.at("index").get<std::uint32_t>()],
                    };

                    std::pmr::vector<std::byte> const descriptor_set_data = write_command_data(temporaries_allocator, Bind_descriptor_sets::Bind_type::Pipeline_resource, pipeline_resource);
                    data.insert(data.end(), descriptor_set_data.begin(), descriptor_set_data.end());
                }
            }

            if (json.contains("dynamic_offsets"))
            {
                for (nlohmann::json const& dynamic_offset_json : json.at("dynamic_offsets"))
                {
                    std::uint32_t const dynamic_offset = dynamic_offset_json.get<std::uint32_t>();

                    std::array<std::byte, sizeof(std::uint32_t)> dynamic_offset_bytes;
                    std::memcpy(dynamic_offset_bytes.data(), &dynamic_offset, dynamic_offset_bytes.size());

                    data.insert(data.end(), dynamic_offset_bytes.begin(), dynamic_offset_bytes.end());
                }
            }

            std::pmr::vector<std::byte> output{ output_allocator };
            output.assign(data.begin(), data.end());
            return output;
        }

        namespace Begin_render_pass
        {
            enum class Type : std::uint8_t
            {
                Dependent
            };

            struct Dependent
            {
                vk::RenderPass render_pass;
            };
        }

        std::pmr::vector<std::byte> create_begin_render_pass_data(
            nlohmann::json const& json,
            std::span<vk::RenderPass const> const render_passes,
            std::pmr::polymorphic_allocator<> const& output_allocator
        ) noexcept
        {
            std::string const& type = json.at("subtype").get<std::string>();
            assert(type == "Dependent");

            Begin_render_pass::Dependent const dependent
            {
                .render_pass = render_passes[json.at("render_pass").get<std::size_t>()]
            };

            Command_type constexpr command_type = Command_type::Begin_render_pass;
            Begin_render_pass::Type constexpr begin_render_pass_type = Begin_render_pass::Type::Dependent;

            std::pmr::vector<std::byte> data{ output_allocator };
            data.resize(sizeof(command_type) + sizeof(begin_render_pass_type) + sizeof(dependent));
            std::memcpy(data.data(), &command_type, sizeof(command_type));
            std::memcpy(data.data() + sizeof(command_type), &begin_render_pass_type, sizeof(begin_render_pass_type));
            std::memcpy(data.data() + sizeof(command_type) + sizeof(begin_render_pass_type), &dependent, sizeof(dependent));
            return data;
        }

        struct Bind_pipeline
        {
            vk::PipelineBindPoint bind_point;
            vk::Pipeline pipeline;
        };

        std::pmr::vector<std::byte> create_bind_pipeline_data(
            nlohmann::json const& json,
            std::span<vk::Pipeline const> const pipelines,
            std::pmr::polymorphic_allocator<> const& output_allocator
        ) noexcept
        {
            assert(json.at("type").get<std::string>() == "Bind_pipeline");

            Command_type constexpr command_type = Command_type::Bind_pipeline;

            Bind_pipeline const command
            {
                .bind_point = json.at("pipeline_bind_point").get<vk::PipelineBindPoint>(),
                .pipeline = pipelines[json.at("pipeline").get<std::size_t>()]
            };

            std::pmr::vector<std::byte> data{ output_allocator };
            data.resize(sizeof(command_type) + sizeof(command));
            std::memcpy(data.data(), &command_type, sizeof(command_type));
            std::memcpy(data.data() + sizeof(command_type), &command, sizeof(command));
            return data;
        }

        namespace Clear_color_image
        {
            enum class Type : std::uint8_t
            {
                Dependent
            };

            struct Dependent
            {
                vk::ClearColorValue clear_color_value;
            };
        }

        vk::ClearColorValue create_color_color_value(
            nlohmann::json const& clear_color_value_json
        ) noexcept
        {
            std::string const& type = clear_color_value_json.at("type").get<std::string>();
            assert(type == "INT" || type == "UINT" || type == "FLOAT");

            nlohmann::json const& values_json = clear_color_value_json.at("values");

            if (type == "FLOAT")
            {
                std::array<float, 4> const values =
                {
                    values_json[0].get<float>(),
                    values_json[1].get<float>(),
                    values_json[2].get<float>(),
                    values_json[3].get<float>(),
                };

                return { values };
            }
            else if (type == "INT")
            {
                std::array<std::int32_t, 4> const values =
                {
                    values_json[0].get<std::int32_t>(),
                    values_json[1].get<std::int32_t>(),
                    values_json[2].get<std::int32_t>(),
                    values_json[3].get<std::int32_t>(),
                };

                return { values };
            }
            else
            {
                assert(type == "UINT");

                std::array<std::uint32_t, 4> const values =
                {
                    values_json[0].get<std::uint32_t>(),
                    values_json[1].get<std::uint32_t>(),
                    values_json[2].get<std::uint32_t>(),
                    values_json[3].get<std::uint32_t>(),
                };

                return { values };
            }
        }

        std::pmr::vector<std::byte> create_color_image_data(
            nlohmann::json const& command_json,
            std::pmr::polymorphic_allocator<std::byte> const& output_allocator
        ) noexcept
        {
            std::string const& subtype = command_json.at("subtype").get<std::string>();
            assert(subtype == "Dependent");

            Clear_color_image::Dependent const dependent
            {
                .clear_color_value = create_color_color_value(command_json.at("clear_color_value"))
            };

            Command_type constexpr command_type = Command_type::Clear_color_image;
            Clear_color_image::Type constexpr clear_subtype = Clear_color_image::Type::Dependent;

            std::pmr::vector<std::byte> data{ output_allocator };
            data.resize(sizeof(Command_type) + sizeof(Clear_color_image::Type) + sizeof(Clear_color_image::Dependent));
            std::memcpy(data.data(), &command_type, sizeof(command_type));
            std::memcpy(data.data() + sizeof(command_type), &clear_subtype, sizeof(clear_subtype));
            std::memcpy(data.data() + sizeof(command_type) + sizeof(clear_subtype), &dependent, sizeof(dependent));
            return data;
        }

        struct Draw
        {
            std::uint32_t vertex_count;
            std::uint32_t instance_count;
            std::uint32_t first_vertex;
            std::uint32_t first_instance;
        };

        std::pmr::vector<std::byte> create_draw_data(
            nlohmann::json const& json,
            std::pmr::polymorphic_allocator<> const& output_allocator
        ) noexcept
        {
            assert(json.at("type").get<std::string>() == "Draw");

            Command_type constexpr command_type = Command_type::Draw;

            Draw const command
            {
                .vertex_count = json.at("vertex_count").get<std::uint32_t>(),
                .instance_count = json.at("instance_count").get<std::uint32_t>(),
                .first_vertex = json.at("first_vertex").get<std::uint32_t>(),
                .first_instance = json.at("first_instance").get<std::uint32_t>(),
            };

            std::pmr::vector<std::byte> data{ output_allocator };
            data.resize(sizeof(command_type) + sizeof(command));
            std::memcpy(data.data(), &command_type, sizeof(command_type));
            std::memcpy(data.data() + sizeof(command_type), &command, sizeof(command));
            return data;
        }

        std::pmr::vector<std::byte> create_end_render_pass_data(
            nlohmann::json const& json,
            std::pmr::polymorphic_allocator<> const& output_allocator
        ) noexcept
        {
            assert(json.at("type").get<std::string>() == "End_render_pass");

            Command_type constexpr command_type = Command_type::End_render_pass;

            std::pmr::vector<std::byte> data{ output_allocator };
            data.resize(sizeof(command_type));
            std::memcpy(data.data(), &command_type, sizeof(command_type));
            return data;
        }

        namespace Image_memory_barrier
        {
            enum class Type : std::uint8_t
            {
                Dependent
            };

            struct Dependent
            {
                vk::AccessFlags source_access_mask;
                vk::AccessFlags destination_access_mask;
                vk::ImageLayout old_layout;
                vk::ImageLayout new_layout;
            };
        }

        struct Pipeline_barrier
        {
            vk::PipelineStageFlagBits source_stage_mask;
            vk::PipelineStageFlagBits destination_stage_mask;
            vk::DependencyFlagBits dependency_flags;
            std::uint8_t memory_barrier_count;
            std::uint8_t buffer_barrier_count;
            std::uint8_t image_barrier_count;
        };

        std::pmr::vector<std::byte> create_image_memory_barrier(
            nlohmann::json const& image_barrier_json,
            std::pmr::polymorphic_allocator<std::byte> const& output_allocator
        ) noexcept
        {
            std::string const& type = image_barrier_json.at("type").get<std::string>();
            assert(type == "Dependent");

            Image_memory_barrier::Dependent const dependent
            {
                .source_access_mask = image_barrier_json.at("source_access_mask").get<vk::AccessFlags>(),
                .destination_access_mask = image_barrier_json.at("destination_access_mask").get<vk::AccessFlags>(),
                .old_layout = image_barrier_json.at("old_layout").get<vk::ImageLayout>(),
                .new_layout = image_barrier_json.at("new_layout").get<vk::ImageLayout>(),
            };

            Image_memory_barrier::Type constexpr barrier_type = Image_memory_barrier::Type::Dependent;

            std::pmr::vector<std::byte> data{ output_allocator };
            data.resize(sizeof(Image_memory_barrier::Type) + sizeof(Image_memory_barrier::Dependent));
            std::memcpy(data.data(), &barrier_type, sizeof(barrier_type));
            std::memcpy(data.data() + sizeof(Image_memory_barrier::Type), &dependent, sizeof(dependent));
            return data;
        }

        std::pmr::vector<std::byte> create_pipeline_barrier(
            nlohmann::json const& command_json,
            std::pmr::polymorphic_allocator<std::byte> const& output_allocator,
            std::pmr::polymorphic_allocator<std::byte> const& temporaries_allocator
        ) noexcept
        {
            Pipeline_barrier const pipeline_barrier
            {
                .source_stage_mask = command_json.at("source_stage_mask").get<vk::PipelineStageFlagBits>(),
                .destination_stage_mask = command_json.at("destination_stage_mask").get<vk::PipelineStageFlagBits>(),
                .dependency_flags = command_json.at("dependency_flags").get<vk::DependencyFlagBits>(),
                .memory_barrier_count = static_cast<std::uint8_t>(command_json.at("memory_barriers").size()),
                .buffer_barrier_count = static_cast<std::uint8_t>(command_json.at("buffer_barriers").size()),
                .image_barrier_count = static_cast<std::uint8_t>(command_json.at("image_barriers").size())
            };

            Command_type constexpr command_type = Command_type::Pipeline_barrier;

            std::pmr::vector<std::byte> data{ output_allocator };
            data.resize(sizeof(Command_type) + sizeof(Pipeline_barrier));
            std::memcpy(data.data(), &command_type, sizeof(command_type));
            std::memcpy(data.data() + sizeof(Command_type), &pipeline_barrier, sizeof(pipeline_barrier));

            for (nlohmann::json const& image_barrier_json : command_json.at("image_barriers"))
            {
                std::pmr::vector<std::byte> const image_memory_barrier_data =
                    create_image_memory_barrier(image_barrier_json, temporaries_allocator);

                data.insert(data.end(), image_memory_barrier_data.begin(), image_memory_barrier_data.end());
            }

            return data;
        }

        std::pmr::vector<std::byte> create_set_screen_viewport_and_scissors_data(
            nlohmann::json const& json,
            std::pmr::polymorphic_allocator<> const& output_allocator
        ) noexcept
        {
            assert(json.at("type").get<std::string>() == "Set_screen_viewport_and_scissors");

            Command_type constexpr command_type = Command_type::Set_screen_viewport_and_scissors;

            std::pmr::vector<std::byte> data{ output_allocator };
            data.resize(sizeof(command_type));
            std::memcpy(data.data(), &command_type, sizeof(command_type));
            return data;
        }

        struct Trace_rays
        {
            std::uint32_t width = 0;
            std::uint32_t height = 0;
            std::uint32_t depth = 0;
            vk::StridedDeviceAddressRegionKHR raygen_shader_binding_table = {};
            vk::StridedDeviceAddressRegionKHR miss_shader_binding_table = {};
            vk::StridedDeviceAddressRegionKHR hit_shader_binding_table = {};
            vk::StridedDeviceAddressRegionKHR callable_shader_binding_table = {};
        };

        std::pmr::vector<std::byte> create_trace_rays_data(
            nlohmann::json const& command_json,
            std::span<vk::StridedDeviceAddressRegionKHR const> const shader_binding_tables,
            std::pmr::polymorphic_allocator<> const& output_allocator
        )
        {
            assert(command_json.at("type").get<std::string>() == "Trace_rays");

            vk::StridedDeviceAddressRegionKHR const raygen = command_json.contains("raygen_shader_binding_table_index") ? shader_binding_tables[command_json.at("raygen_shader_binding_table_index").get<std::size_t>()] : vk::StridedDeviceAddressRegionKHR{};
            vk::StridedDeviceAddressRegionKHR const miss = command_json.contains("miss_shader_binding_table_index") ? shader_binding_tables[command_json.at("miss_shader_binding_table_index").get<std::size_t>()] : vk::StridedDeviceAddressRegionKHR{};
            vk::StridedDeviceAddressRegionKHR const hit = command_json.contains("hit_shader_binding_table_index") ? shader_binding_tables[command_json.at("hit_shader_binding_table_index").get<std::size_t>()] : vk::StridedDeviceAddressRegionKHR{};
            vk::StridedDeviceAddressRegionKHR const callable = command_json.contains("callable_shader_binding_table_index") ? shader_binding_tables[command_json.at("callable_shader_binding_table_index").get<std::size_t>()] : vk::StridedDeviceAddressRegionKHR{};

            Trace_rays const trace_rays
            {
                .width = command_json.at("width").get<std::uint32_t>(),
                    .height = command_json.at("height").get<std::uint32_t>(),
                    .depth = command_json.at("depth").get<std::uint32_t>(),
                    .raygen_shader_binding_table = raygen,
                    .miss_shader_binding_table = miss,
                    .hit_shader_binding_table = hit,
                    .callable_shader_binding_table = callable,
            };

            Command_type constexpr command_type = Command_type::Trace_rays;

            std::pmr::vector<std::byte> data{ output_allocator };
            data.resize(sizeof(command_type) + sizeof(Trace_rays));
            std::memcpy(data.data(), &command_type, sizeof(command_type));
            std::memcpy(data.data() + sizeof(command_type), &trace_rays, sizeof(trace_rays));
            return data;
        }

        std::pmr::vector<std::byte> create_command_data(
            nlohmann::json const& command_json,
            std::span<vk::DescriptorSet const> const descriptor_sets,
            std::span<vk::Pipeline const> const pipelines,
            std::span<vk::PipelineLayout const> const pipeline_layouts,
            std::span<vk::RenderPass const> const render_passes,
            std::span<vk::StridedDeviceAddressRegionKHR const> const shader_binding_tables,
            std::pmr::polymorphic_allocator<std::byte> const& output_allocator,
            std::pmr::polymorphic_allocator<std::byte> const& temporaries_allocator
        ) noexcept
        {
            assert(command_json.contains("type"));
            std::string const& type = command_json.at("type").get<std::string>();

            if (type == "Begin_render_pass")
            {
                return create_begin_render_pass_data(command_json, render_passes, output_allocator);
            }
            else if (type == "Bind_descriptor_sets")
            {
                return create_bind_descriptor_sets_data(command_json, descriptor_sets, pipeline_layouts, output_allocator, temporaries_allocator);
            }
            else if (type == "Bind_pipeline")
            {
                return create_bind_pipeline_data(command_json, pipelines, output_allocator);
            }
            else if (type == "Clear_color_image")
            {
                return create_color_image_data(command_json, output_allocator);
            }
            else if (type == "Draw")
            {
                return create_draw_data(command_json, output_allocator);
            }
            else if (type == "End_render_pass")
            {
                return create_end_render_pass_data(command_json, output_allocator);
            }
            else if (type == "Pipeline_barrier")
            {
                return create_pipeline_barrier(command_json, output_allocator, temporaries_allocator);
            }
            else if (type == "Trace_rays")
            {
                return create_trace_rays_data(command_json, shader_binding_tables, output_allocator);
            }
            else if (type == "Set_screen_viewport_and_scissors")
            {
                return create_set_screen_viewport_and_scissors_data(command_json, output_allocator);
            }
            else
            {
                assert(false && "Command not recognized!");
                return {};
            }
        }

        template <class Type>
        Type read(std::byte const* const source) noexcept
        {
            Type destination = {};
            std::memcpy(&destination, source, sizeof(Type));

            return destination;
        }
    }

    Commands_data create_commands_data(
        nlohmann::json const& commands_json,
        std::span<vk::DescriptorSet const> const descriptor_sets,
        std::span<vk::Pipeline const> const pipelines,
        std::span<vk::PipelineLayout const> const pipeline_layouts,
        std::span<vk::RenderPass const> const render_passes,
        std::span<vk::StridedDeviceAddressRegionKHR const> const shader_binding_tables,
        std::pmr::polymorphic_allocator<std::byte> const& output_allocator,
        std::pmr::polymorphic_allocator<std::byte> const& temporaries_allocator
    ) noexcept
    {
        std::pmr::vector<std::byte> commands_data{ output_allocator };

        for (nlohmann::json const& command_json : commands_json)
        {
            std::pmr::vector<std::byte> const command_data = create_command_data(command_json, descriptor_sets, pipelines, pipeline_layouts, render_passes, shader_binding_tables, temporaries_allocator, temporaries_allocator);

            commands_data.insert(commands_data.end(), command_data.begin(), command_data.end());
        }

        return { commands_data };
    }

    namespace
    {
        using Commands_data_offset = std::size_t;

        std::span<std::byte const> create_data_span(
            std::span<std::byte const> const bytes,
            std::size_t commands_data_offset
        ) noexcept
        {
            return { bytes.data() + commands_data_offset, bytes.size() - commands_data_offset };
        }

        template <typename T>
        struct Read_data
        {
            T data = {};
            std::size_t read_bytes = {};
        };

        Commands_data_offset add_begin_render_pass_command(
            vk::CommandBuffer const command_buffer,
            std::span<vk::Framebuffer const> const framebuffers,
            std::span<vk::Rect2D const> const framebuffer_render_areas,
            std::span<std::byte const> const bytes
        ) noexcept
        {
            Commands_data_offset commands_data_offset = 0;

            vk::Framebuffer const framebuffer = framebuffers[0]; // TODO
            vk::Rect2D const& framebuffer_render_area = framebuffer_render_areas[0]; // TODO

            Begin_render_pass::Type const subtype = read<Begin_render_pass::Type>(bytes.data() + commands_data_offset);
            commands_data_offset += sizeof(subtype);

            assert(subtype == Begin_render_pass::Type::Dependent);

            Begin_render_pass::Dependent const command = read<Begin_render_pass::Dependent>(bytes.data() + commands_data_offset);
            commands_data_offset += sizeof(command);

            vk::RenderPassBeginInfo const begin_info
            {
                .renderPass = command.render_pass,
                .framebuffer = framebuffer,
                .renderArea = framebuffer_render_area,
                .clearValueCount = 0,
                .pClearValues = nullptr,
            };

            command_buffer.beginRenderPass(
                begin_info,
                vk::SubpassContents::eInline
            );

            return commands_data_offset;
        }

        Read_data<std::pmr::vector<vk::DescriptorSet>> get_command_descriptor_sets(
            std::span<std::byte const> const bytes,
            std::uint32_t const descriptor_set_count,
            std::span<vk::DescriptorSet const> const frame_descriptor_sets,
            std::pmr::polymorphic_allocator<std::byte> const& output_allocator
        )
        {
            if (descriptor_set_count == 0)
            {
                return { .data = std::pmr::vector<vk::DescriptorSet>{ output_allocator }, .read_bytes = 0 };
            }

            Commands_data_offset commands_data_offset = 0;

            std::pmr::vector<vk::DescriptorSet> descriptor_sets{ output_allocator };
            descriptor_sets.reserve(descriptor_set_count);

            for (std::uint32_t descriptor_set_index = 0; descriptor_set_index < descriptor_set_count; ++descriptor_set_index)
            {
                Bind_descriptor_sets::Bind_type const bind_type = read<Bind_descriptor_sets::Bind_type>(bytes.data() + commands_data_offset);
                commands_data_offset += sizeof(bind_type);

                if (bind_type == Bind_descriptor_sets::Bind_type::Frame_resource)
                {
                    Bind_descriptor_sets::Frame_resource const frame_resource = read<Bind_descriptor_sets::Frame_resource>(bytes.data() + commands_data_offset);
                    commands_data_offset += sizeof(frame_resource);

                    assert(frame_resource.descriptor_set_index < frame_descriptor_sets.size());
                    descriptor_sets.push_back(frame_descriptor_sets[frame_resource.descriptor_set_index]);
                }
                else
                {
                    assert(bind_type == Bind_descriptor_sets::Bind_type::Pipeline_resource);

                    Bind_descriptor_sets::Pipeline_resource const pipeline_resource = read<Bind_descriptor_sets::Pipeline_resource>(bytes.data() + commands_data_offset);
                    commands_data_offset += sizeof(pipeline_resource);

                    descriptor_sets.push_back(pipeline_resource.descriptor_set);
                }

                assert(commands_data_offset <= bytes.size_bytes());
            }

            assert(descriptor_sets.size() == descriptor_set_count);
            return { .data = std::move(descriptor_sets), .read_bytes = commands_data_offset, };
        }

        Read_data<std::pmr::vector<std::uint32_t>> get_command_dynamic_offsets(
            std::span<std::byte const> const bytes,
            std::uint32_t const dynamic_offset_count,
            std::pmr::polymorphic_allocator<std::byte> const& output_allocator
        )
        {
            assert(bytes.size_bytes() <= (dynamic_offset_count * sizeof(std::uint32_t)));

            if (dynamic_offset_count == 0)
            {
                return { .data = std::pmr::vector<std::uint32_t>{ output_allocator }, .read_bytes = 0 };
            }

            Commands_data_offset const bytes_to_read = dynamic_offset_count * sizeof(std::uint32_t);

            std::pmr::vector<std::uint32_t> dynamic_offsets{ output_allocator };
            dynamic_offsets.resize(dynamic_offset_count);

            std::memcpy(dynamic_offsets.data(), bytes.data(), bytes_to_read);

            assert(dynamic_offsets.size() == dynamic_offset_count);
            return { .data = std::move(dynamic_offsets), .read_bytes = bytes_to_read, };
        }

        Commands_data_offset add_bind_descriptor_sets_command(
            vk::CommandBuffer const command_buffer,
            std::span<vk::DescriptorSet const> const frame_descriptor_sets,
            std::span<std::byte const> const bytes,
            std::pmr::polymorphic_allocator<std::byte> const& temporaries_allocator
        )
        {
            Commands_data_offset commands_data_offset = 0;

            Bind_descriptor_sets const command = read<Bind_descriptor_sets>(bytes.data() + commands_data_offset);
            commands_data_offset += sizeof(command);

            Read_data<std::pmr::vector<vk::DescriptorSet>> const descriptor_sets = get_command_descriptor_sets(
                create_data_span(bytes, commands_data_offset),
                command.descriptor_set_count,
                frame_descriptor_sets,
                temporaries_allocator
            );
            commands_data_offset += descriptor_sets.read_bytes;

            Read_data<std::pmr::vector<std::uint32_t>> const dynamic_offsets = get_command_dynamic_offsets(
                create_data_span(bytes, commands_data_offset),
                command.dynamic_offset_count,
                temporaries_allocator
            );
            commands_data_offset += dynamic_offsets.read_bytes;

            command_buffer.bindDescriptorSets(
                command.pipeline_bind_point,
                command.pipeline_layout,
                command.first_set,
                command.descriptor_set_count,
                descriptor_sets.data.data(),
                command.dynamic_offset_count,
                dynamic_offsets.data.data()
            );

            return commands_data_offset;
        }

        Commands_data_offset add_bind_pipeline_command(
            vk::CommandBuffer const command_buffer,
            std::span<std::byte const> const bytes
        ) noexcept
        {
            Commands_data_offset commands_data_offset = 0;

            Bind_pipeline const command = read<Bind_pipeline>(bytes.data() + commands_data_offset);
            commands_data_offset += sizeof(command);

            command_buffer.bindPipeline(
                command.bind_point,
                command.pipeline
            );

            return commands_data_offset;
        }

        Commands_data_offset add_clear_color_image_command(
            vk::CommandBuffer const command_buffer,
            std::span<vk::Image const> const images,
            std::span<vk::ImageSubresourceRange const> const image_subresource_ranges,
            std::span<std::byte const> const bytes
        ) noexcept
        {
            Commands_data_offset commands_data_offset = 0;

            vk::Image const image = images[0]; // TODO
            vk::ImageSubresourceRange const& image_subresource_range = image_subresource_ranges[0]; // TODO

            Clear_color_image::Type const clear_subtype = read<Clear_color_image::Type>(bytes.data() + commands_data_offset);
            commands_data_offset += sizeof(Clear_color_image::Type);

            assert(clear_subtype == Clear_color_image::Type::Dependent);

            Clear_color_image::Dependent const command = read<Clear_color_image::Dependent>(bytes.data() + commands_data_offset);
            commands_data_offset += sizeof(Clear_color_image::Dependent);

            command_buffer.clearColorImage(
                image,
                vk::ImageLayout::eTransferDstOptimal,
                &command.clear_color_value,
                1,
                &image_subresource_range
            );

            return commands_data_offset;
        }

        Commands_data_offset add_draw_command(
            vk::CommandBuffer const command_buffer,
            std::span<std::byte const> const bytes
        ) noexcept
        {
            Commands_data_offset commands_data_offset = 0;

            Draw const command = read<Draw>(bytes.data() + commands_data_offset);
            commands_data_offset += sizeof(command);

            command_buffer.draw(
                command.vertex_count,
                command.instance_count,
                command.first_vertex,
                command.first_instance
            );

            return commands_data_offset;
        }

        Commands_data_offset add_end_render_pass_command(
            vk::CommandBuffer const command_buffer
        ) noexcept
        {
            command_buffer.endRenderPass();

            return 0;
        }

        std::pair<Commands_data_offset, std::pmr::vector<vk::ImageMemoryBarrier>> create_image_memory_barriers(
            std::span<std::byte const> const bytes,
            std::uint8_t const barrier_count,
            std::span<vk::Image const> const images,
            std::span<vk::ImageSubresourceRange const> const image_subresource_ranges,
            std::pmr::polymorphic_allocator<> const& output_allocator
        ) noexcept
        {
            Commands_data_offset commands_data_offset = 0;

            vk::Image const image = images[0]; // TODO
            vk::ImageSubresourceRange const& image_subresource_range = image_subresource_ranges[0]; // TODO

            std::pmr::vector<vk::ImageMemoryBarrier> barriers{ output_allocator };
            barriers.reserve(barrier_count);

            for (std::uint8_t barrier_index = 0; barrier_index < barrier_count; ++barrier_index)
            {
                Image_memory_barrier::Type const type = read<Image_memory_barrier::Type>(bytes.data() + commands_data_offset);
                commands_data_offset += sizeof(Image_memory_barrier::Type);

                assert(type == Image_memory_barrier::Type::Dependent);

                Image_memory_barrier::Dependent const command = read<Image_memory_barrier::Dependent>(bytes.data() + commands_data_offset);
                commands_data_offset += sizeof(Image_memory_barrier::Dependent);

                barriers.push_back({
                    .srcAccessMask = command.source_access_mask,
                    .dstAccessMask = command.destination_access_mask,
                    .oldLayout = command.old_layout,
                    .newLayout = command.new_layout,
                    .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                    .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                    .image = image,
                    .subresourceRange = image_subresource_range,
                    });
            }

            return { commands_data_offset, barriers };
        }

        Commands_data_offset add_pipeline_barrier_command(
            vk::CommandBuffer const command_buffer,
            std::span<vk::Image const> const images,
            std::span<vk::ImageSubresourceRange const> const image_subresource_ranges,
            std::span<std::byte const> const bytes,
            std::pmr::polymorphic_allocator<> const& temporaries_allocator
        ) noexcept
        {
            Commands_data_offset commands_data_offset = 0;

            vk::Image const image = images[0]; // TODO
            vk::ImageSubresourceRange const& image_subresource_range = image_subresource_ranges[0]; // TODO

            Pipeline_barrier const command = read<Pipeline_barrier>(bytes.data() + commands_data_offset);
            commands_data_offset += sizeof(Pipeline_barrier);

            assert(command.memory_barrier_count == 0);
            assert(command.buffer_barrier_count == 0);

            std::pair<Commands_data_offset, std::pmr::vector<vk::ImageMemoryBarrier>> const image_barriers =
                create_image_memory_barriers(
                    { bytes.data() + commands_data_offset, bytes.size() - commands_data_offset },
                    command.image_barrier_count,
                    images,
                    image_subresource_ranges,
                    temporaries_allocator
                );
            commands_data_offset += image_barriers.first;

            command_buffer.pipelineBarrier(
                command.source_stage_mask,
                command.destination_stage_mask,
                command.dependency_flags,
                0,
                nullptr,
                0,
                nullptr,
                static_cast<std::uint32_t>(image_barriers.second.size()),
                image_barriers.second.data()
            );
            // TODO use synchronization_2

            return commands_data_offset;
        }

        Commands_data_offset add_set_screen_viewport_and_scissors_commands(
            vk::CommandBuffer const command_buffer,
            std::span<vk::Rect2D const> const render_areas
        ) noexcept
        {
            vk::Rect2D const& render_area = render_areas[0]; // TODO

            {
                std::array<vk::Viewport, 1> const viewports
                {
                    vk::Viewport
                    {
                        .x = static_cast<float>(render_area.offset.x),
                        .y = static_cast<float>(render_area.offset.y),
                        .width = static_cast<float>(render_area.extent.width),
                        .height = static_cast<float>(render_area.extent.height),
                        .minDepth = 0.0f,
                        .maxDepth = 1.0f,
                    }
                };

                command_buffer.setViewport(0, viewports);
            }

            {
                std::array<vk::Rect2D, 1> const scissors
                {
                    vk::Rect2D
                    {
                        .offset = render_area.offset,
                        .extent = render_area.extent,
                    }
                };

                command_buffer.setScissor(0, scissors);
            }

            return 0;
        }

        Commands_data_offset add_trace_rays_command(
            vk::CommandBuffer const command_buffer,
            std::span<std::byte const> const bytes
        ) noexcept
        {
            Commands_data_offset commands_data_offset = 0;

            Trace_rays const command = read<Trace_rays>(bytes.data() + commands_data_offset);
            commands_data_offset += sizeof(Trace_rays);

            command_buffer.traceRaysKHR(
                &command.raygen_shader_binding_table,
                &command.miss_shader_binding_table,
                &command.hit_shader_binding_table,
                &command.callable_shader_binding_table,
                command.width,
                command.height,
                command.depth
            );

            return commands_data_offset;
        }
    }

    void draw(
        vk::CommandBuffer const command_buffer,
        std::span<vk::Buffer const> const output_buffers,
        std::span<vk::Image const> const output_images,
        std::span<vk::ImageView const> const output_image_views,
        std::span<vk::ImageSubresourceRange const> const output_image_subresource_ranges,
        std::span<vk::DescriptorSet const> const frame_descriptor_sets,
        std::span<vk::Framebuffer const> const output_framebuffers,
        std::span<vk::Rect2D const> const output_render_areas,
        Commands_data const& commands_data,
        std::pmr::polymorphic_allocator<> const& temporaries_allocator
    ) noexcept
    {
        std::span<std::byte const> const bytes = commands_data.bytes;

        Commands_data_offset offset_in_bytes = 0;

        while (offset_in_bytes < bytes.size())
        {
            Command_type const command_type = read<Command_type>(bytes.data() + offset_in_bytes);
            offset_in_bytes += sizeof(Command_type);

            std::span<std::byte const> const next_command_bytes = { bytes.data() + offset_in_bytes, bytes.size() - offset_in_bytes };

            switch (command_type)
            {
            case Command_type::Begin_render_pass:
                offset_in_bytes += add_begin_render_pass_command(command_buffer, output_framebuffers, output_render_areas, next_command_bytes);
                break;

            case Command_type::Bind_descriptor_sets:
                offset_in_bytes += add_bind_descriptor_sets_command(command_buffer, frame_descriptor_sets, next_command_bytes, temporaries_allocator);
                break;

            case Command_type::Bind_pipeline:
                offset_in_bytes += add_bind_pipeline_command(command_buffer, next_command_bytes);
                break;

            case Command_type::Draw:
                offset_in_bytes += add_draw_command(command_buffer, next_command_bytes);
                break;

            case Command_type::Clear_color_image:
                offset_in_bytes += add_clear_color_image_command(command_buffer, output_images, output_image_subresource_ranges, next_command_bytes);
                break;

            case Command_type::End_render_pass:
                offset_in_bytes += add_end_render_pass_command(command_buffer);
                break;

            case Command_type::Pipeline_barrier:
                offset_in_bytes += add_pipeline_barrier_command(command_buffer, output_images, output_image_subresource_ranges, next_command_bytes, temporaries_allocator);
                break;

            case Command_type::Trace_rays:
                offset_in_bytes += add_trace_rays_command(command_buffer, next_command_bytes);
                break;

            case Command_type::Set_screen_viewport_and_scissors:
                offset_in_bytes += add_set_screen_viewport_and_scissors_commands(command_buffer, output_render_areas);
                break;

            default:
                assert(false && "Unrecognized command!");
                break;
            }
        }
    }
}
