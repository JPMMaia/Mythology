module mythology.core.vulkan;

import maia.renderer.vulkan;

import <vulkan/vulkan.h>;

import <algorithm>;
import <array>;
import <cassert>;
import <cstring>;
import <filesystem>;
import <fstream>;
import <functional>;
import <memory_resource>;
import <optional>;
import <span>;
import <string_view>;
import <vector>;

using namespace Maia::Renderer::Vulkan;

namespace Mythology::Core::Vulkan
{
    VkInstance create_instance(
        std::optional<Application_description> application_description,
        std::optional<Engine_description> engine_description,
        API_version api_version,
        std::span<char const* const> const required_extensions
    ) noexcept
    {
        std::pmr::vector<VkLayerProperties> const layer_properties = enumerate_instance_layer_properties();

        auto const is_layer_to_enable = [](VkLayerProperties const& properties) -> bool
        {
            return 
                std::strcmp(properties.layerName, "VK_LAYER_KHRONOS_validation") == 0;
        };

        auto const get_layer_name = [](VkLayerProperties const& properties)
        {
            return properties.layerName;
        };

        std::pmr::vector<char const*> layers_to_enable;
        layers_to_enable.reserve(5);
        for (std::size_t layer_index = 0; layer_index < layer_properties.size(); ++layer_index)
        {
            if (is_layer_to_enable(layer_properties[layer_index]))
            {
                layers_to_enable.push_back(
                    layer_properties[layer_index].layerName);
            }
        }

        std::pmr::vector<char const*> extensions_to_enable;
        extensions_to_enable.reserve(required_extensions.size() + 5);
        extensions_to_enable.assign(required_extensions.begin(), required_extensions.end());
        extensions_to_enable.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

        std::array<VkValidationFeatureEnableEXT, 2> constexpr validation_features_to_enable
        {
            VK_VALIDATION_FEATURE_ENABLE_BEST_PRACTICES_EXT
        };
        
        VkValidationFeaturesEXT const validation_features
        {
            .sType = VK_STRUCTURE_TYPE_VALIDATION_FEATURES_EXT,
            .pNext = nullptr,
            .enabledValidationFeatureCount = validation_features_to_enable.size(),
            .pEnabledValidationFeatures = validation_features_to_enable.data(),
            .disabledValidationFeatureCount = 0,
            .pDisabledValidationFeatures = nullptr
        };

        return Maia::Renderer::Vulkan::create_instance(
            application_description,
            engine_description,
            api_version,
            layers_to_enable,
            extensions_to_enable,
            &validation_features);
    }

    Physical_device select_physical_device(VkInstance const instance) noexcept
    {
        std::pmr::vector<Physical_device> const physical_devices = enumerate_physical_devices(instance);

        return physical_devices[0];
    }

        Queue_family_index find_graphics_queue_family_index(
        Physical_device const physical_device
    ) noexcept
    {
        std::pmr::vector<Queue_family_properties> const queue_family_properties = 
            get_physical_device_queue_family_properties(physical_device);

        std::optional<Queue_family_index> const queue_family_index = find_queue_family_with_capabilities(
            queue_family_properties,
            [](Queue_family_properties const& properties) -> bool { return has_graphics_capabilities(properties); }
        );

        assert(queue_family_index.has_value());

        return *queue_family_index;
    }

    Queue_family_index find_present_queue_family_index(
        Physical_device const physical_device,
        Surface const surface,
        std::optional<Queue_family_index> const preference
    ) noexcept
    {
        if (preference)
        {
            if (is_surface_supported(physical_device, *preference, surface))
            {
                return *preference;
            }
        }

        std::uint32_t const queue_family_count = 
            get_physical_device_queue_family_count(physical_device);

        for (std::uint32_t index = 0; index < queue_family_count; ++index)
        {
            if (is_surface_supported(physical_device, {index}, surface))
            {
                return {index};
            } 
        }

        assert(false);
    }

    namespace
    {
        std::pmr::vector<char const*> select_device_extensions(
            Physical_device const physical_device,
            std::function<bool(VkExtensionProperties)> const& is_extension_to_enable
        ) noexcept
        {
            std::pmr::vector<VkExtensionProperties> const extensions_properties = 
                enumerate_physical_device_extension_properties(physical_device, {});

            std::pmr::vector<char const*> selected_extensions_properties;
            selected_extensions_properties.reserve(1);

            for (VkExtensionProperties const& properties : extensions_properties)
            {
                if (is_extension_to_enable(properties))
                {
                    selected_extensions_properties.push_back(properties.extensionName);
                }
            }

            return selected_extensions_properties;
        }
    }

    Device create_device(
        Physical_device const physical_device,
        std::span<Queue_family_index const> const queue_family_indices,
        std::function<bool(VkExtensionProperties)> const& is_extension_to_enable) noexcept
    {
        std::array<float, 1> constexpr queue_priorities{1.0f};

        std::pmr::vector<Device_queue_create_info> const queue_create_infos = [queue_family_indices, &queue_priorities]() -> std::pmr::vector<Device_queue_create_info>
        {
            std::pmr::vector<Device_queue_create_info> queue_create_infos;
            queue_create_infos.resize(queue_family_indices.size());

            std::transform(
                queue_family_indices.begin(),
                queue_family_indices.end(),
                queue_create_infos.begin(),
                [queue_priorities](Queue_family_index const index) -> Device_queue_create_info { return create_device_queue_create_info(index.value, 1, queue_priorities); }
            );

            return queue_create_infos;
        }();

        std::pmr::vector<char const*> const extensions_to_enable = 
            select_device_extensions(physical_device, is_extension_to_enable);

        return create_device(physical_device, queue_create_infos, extensions_to_enable);
    }

    struct Memory_type_info
    {
        VkMemoryPropertyFlags memory_properties;
        Memory_type_index memory_type_index;
    };

    Memory_type_info get_memory_type_info(
        Physical_device_memory_properties const& physical_device_memory_properties,
        Memory_requirements const& memory_requirements
    ) noexcept
    {
        Memory_type_bits const memory_type_bits = get_memory_type_bits(memory_requirements);
        
        std::array<VkMemoryPropertyFlags, 3> constexpr properties_sorted_by_preference
        {
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
        };

        for (VkMemoryPropertyFlags const properties : properties_sorted_by_preference)
        {
            std::optional<Memory_type_index_and_properties> const memory_type_index_and_properties = find_memory_type(
                physical_device_memory_properties,
                memory_type_bits,
                properties
            );

            if (memory_type_index_and_properties)
            {
                return {properties, memory_type_index_and_properties->type_index};
            }
        }

        assert(false);
        return {};
    }

    Device_memory_and_color_image create_device_memory_and_color_image(
        Physical_device const physical_device,
        Device const device,
        VkFormat const format,
        VkExtent3D const extent
    ) noexcept
    {
        Image const color_image = create_image(
            device,
            {},
            VK_IMAGE_TYPE_2D,
            format,
            extent,
            Mip_level_count{1},
            Array_layer_count{1},
            VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
            VK_IMAGE_LAYOUT_UNDEFINED,
            VK_SAMPLE_COUNT_1_BIT,
            {},
            VK_IMAGE_TILING_LINEAR
        );

        Physical_device_memory_properties const physical_device_memory_properties = get_phisical_device_memory_properties(physical_device);
        Memory_requirements const color_image_memory_requirements = get_memory_requirements(device.value, color_image.value);
        Memory_type_info const color_image_memory_type_info = get_memory_type_info(physical_device_memory_properties, color_image_memory_requirements);

        VkDeviceMemory const device_memory =
            allocate_memory(device.value, color_image_memory_requirements.value.size, color_image_memory_type_info.memory_type_index, {});
        bind_memory(device.value, color_image.value, device_memory, 0);

        return {device_memory, color_image};
    }

    Render_pass create_render_pass(Device const device, VkFormat const color_image_format) noexcept
    {
        VkAttachmentDescription const color_attachment_description
        {
            .flags = {},
            .format = color_image_format,
            .samples = VK_SAMPLE_COUNT_1_BIT,
            .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
            .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
            .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
            .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
            .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
            .finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        };

        VkAttachmentReference const color_attachment_reference
        {
            0,
            VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
        };

        VkSubpassDescription const subpass_description
        {
            .flags = {},
            .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
            .inputAttachmentCount = 0,
            .pInputAttachments = nullptr,
            .colorAttachmentCount = 1,
            .pColorAttachments = &color_attachment_reference,
            .pResolveAttachments = nullptr,
            .pDepthStencilAttachment = nullptr,
            .preserveAttachmentCount = 0,
            .pPreserveAttachments = nullptr,
        };

        VkSubpassDependency const subpass_dependency
        {
            .srcSubpass = VK_SUBPASS_EXTERNAL,
            .dstSubpass = {},
            .srcStageMask = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
            .dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
            .srcAccessMask = {},
            .dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
            .dependencyFlags = {}
        };

        return create_render_pass(
            device,
            {&color_attachment_description, 1},
            {&subpass_description, 1},
            {&subpass_dependency, 1},
            {}
        );
    }

    VkPipeline create_vertex_and_fragment_pipeline(
        Device const device,
        std::optional<VkPipelineCache> const pipeline_cache,
        VkPipelineLayout const pipeline_layout,
        VkRenderPass const render_pass,
        std::uint32_t const subpass_index,
        std::uint32_t const subpass_attachment_count,
        VkShaderModule const vertex_shader,
        VkShaderModule const fragment_shader
    ) noexcept
    {
        std::array<VkPipelineShaderStageCreateInfo, 2> const shader_stages_create_info =
            create_shader_stages_create_info(vertex_shader, fragment_shader);

        VkPipelineVertexInputStateCreateInfo constexpr vertex_input_state_create_info = 
            create_vertex_input_state_create_info();
        
        VkPipelineInputAssemblyStateCreateInfo constexpr input_assembly_state_create_info = 
            create_input_assembly_state_create_info(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);

        VkPipelineViewportStateCreateInfo constexpr viewport_state_create_info =
            create_viewport_state_create_info(1, 1);

        VkPipelineRasterizationStateCreateInfo const rasterization_state_create_info
        {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
            .pNext = nullptr,
            .flags = {},
            .depthClampEnable = VK_FALSE,
            .rasterizerDiscardEnable = VK_FALSE,
            .polygonMode = VK_POLYGON_MODE_FILL,
            .cullMode = VK_CULL_MODE_BACK_BIT,
            .frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE,
            .depthBiasEnable = VK_FALSE,
            .depthBiasConstantFactor = {},
            .depthBiasClamp = {},
            .depthBiasSlopeFactor = {},
            .lineWidth = 1.0f,
        };

        VkPipelineMultisampleStateCreateInfo const multisample_state_create_info = 
            create_disabled_multisample_state_create_info();

        VkPipelineDepthStencilStateCreateInfo constexpr depth_stencil_state_create_info =
            create_disabled_depth_stencil_state_create_info();

        std::pmr::vector<VkPipelineColorBlendAttachmentState> const color_blend_attachment_states
        (
            subpass_attachment_count,
            create_disabled_color_blend_attachment_state()
        );

        VkPipelineColorBlendStateCreateInfo const color_blend_state_create_info = 
            create_disabled_color_blend_state_create_info(color_blend_attachment_states);

        VkPipelineDynamicStateCreateInfo const dynamic_state_create_info =
            create_viewport_scissor_dynamic_state_create_info();

        VkGraphicsPipelineCreateInfo const graphics_pipeline_create_info
        {
            .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
            .pNext = nullptr,
            .flags = {},
            .stageCount = static_cast<std::uint32_t>(shader_stages_create_info.size()),
            .pStages = shader_stages_create_info.data(),
            .pVertexInputState = &vertex_input_state_create_info,
            .pInputAssemblyState = &input_assembly_state_create_info,
            .pTessellationState = nullptr,
            .pViewportState = &viewport_state_create_info,
            .pRasterizationState = &rasterization_state_create_info,
            .pMultisampleState = &multisample_state_create_info,
            .pDepthStencilState = &depth_stencil_state_create_info,
            .pColorBlendState = &color_blend_state_create_info,
            .pDynamicState = &dynamic_state_create_info,
            .layout = pipeline_layout,
            .renderPass = render_pass,
            .subpass = subpass_index,
            .basePipelineHandle = VK_NULL_HANDLE,
            .basePipelineIndex = -1,
        };

        return create_graphics_pipeline(device.value, graphics_pipeline_create_info, pipeline_cache.has_value() ? *pipeline_cache : VK_NULL_HANDLE);
    }

    void clear_and_begin_render_pass(
        Command_buffer const command_buffer,
        Render_pass const render_pass,
        Framebuffer const framebuffer,
        VkClearColorValue const clear_color,
        Image const output_image,
        VkImageSubresourceRange const output_image_subresource_range,
        VkRect2D const output_render_area
    ) noexcept
    {
        {
            VkImageMemoryBarrier const image_memory_barrier
            {
                .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
                .pNext = nullptr,
                .srcAccessMask = {},
                .dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
                .oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
                .newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                .image = output_image.value,
                .subresourceRange = output_image_subresource_range
            };

            vkCmdPipelineBarrier(
                command_buffer.value,
                VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, 
                VK_PIPELINE_STAGE_TRANSFER_BIT,
                {},
                0,
                nullptr,
                0,
                nullptr,
                1,
                &image_memory_barrier
            );
        }

        {
            VkClearValue const clear_value
            {
                .color = clear_color
            };

            vkCmdClearColorImage(
                command_buffer.value,
                output_image.value,
                VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 
                &clear_value.color, 
                1,
                &output_image_subresource_range
            );
        }

        {
            VkImageMemoryBarrier const image_memory_barrier
            {
                .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
                .pNext = nullptr,
                .srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
                .dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
                .oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                .newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                .image = output_image.value,
                .subresourceRange = output_image_subresource_range
            };

            vkCmdPipelineBarrier(
                command_buffer.value,
                VK_PIPELINE_STAGE_TRANSFER_BIT,
                VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                {},
                0,
                nullptr,
                0,
                nullptr,
                1,
                &image_memory_barrier
            );
        }

        {
            VkClearValue const clear_value
            {
                .color = clear_color
            };

            begin_render_pass(
                command_buffer,
                render_pass,
                framebuffer,
                output_render_area,
                {&clear_value, 1},
                VK_SUBPASS_CONTENTS_INLINE
            );
        }
    }

    void end_render_pass_and_switch_layout(
        Command_buffer const command_buffer,
        Image const output_image,
        VkImageSubresourceRange const output_image_subresource_range,
        bool const switch_to_present_layout
    ) noexcept
    {
        end_render_pass(command_buffer);

        if (switch_to_present_layout)
        {
            VkImageMemoryBarrier const image_memory_barrier
            {
                .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
                .pNext = nullptr,
                .srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
                .dstAccessMask = 0,
                .oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                .newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
                .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                .image = output_image.value,
                .subresourceRange = output_image_subresource_range
            };

            vkCmdPipelineBarrier(
                command_buffer.value,
                VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, 
                VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
                {},
                0,
                nullptr,
                0,
                nullptr,
                1,
                &image_memory_barrier
            );
        }
    }

    void render_old() noexcept
    {
        {
            // ReadDataFromHostVisible(host_visible_memory)
            //   assert(is host_visible)
            //   if not host_coherent then invalidate cache
            //   map
            //   memcpy
            //   unmap

            // WriteDataToHostVisible(host_visible_memory):
            //   assert(is host_visible)
            //   map
            //   memcpy
            //   unmap
            //   if not host_coherent then flush cache

            // WriteDataToDeviceLocal(command_buffer, resource_memory, host_visible_memory):
            //    assert(resource memory is device local)
            //    assert(staged_memory is host visible)
            //    WriteDataToHostVisible(host_visible_memory)
            //    command_buffer() <- copy from host visible to device local

            // Memory_state.
            // std::optional<Memory_range> = find_available_memory(memory_state, image)
            // If Memory_range
            //    bind_memory(device, image, memory_state[index].device_memory, memory_range.offset)
            // else
            //    create new memory
            //    bind_memory()
        }
    }

    std::pmr::vector<std::byte> read_memory(
        Device const device,
        VkDeviceMemory const device_memory,
        VkSubresourceLayout const subresource_layout,
        std::pmr::polymorphic_allocator<std::byte> const& allocator
    ) noexcept
    {
        std::pmr::vector<std::byte> memory_data{allocator};
        memory_data.resize(subresource_layout.size);

        {
            Mapped_memory const mapped_memory{device.value, device_memory, subresource_layout.offset, subresource_layout.size};
            std::memcpy(memory_data.data(), mapped_memory.data(), memory_data.size());
        }

        return memory_data;
    }

    void write_p3(
        std::ostream& output_stream,
        std::span<std::byte const> const data_to_write,
        VkSubresourceLayout const subresource_layout,
        VkExtent3D const subresource_extent
    ) noexcept
    {
        output_stream << "P3\n";
        output_stream << subresource_extent.width << ' ' << subresource_extent.height << '\n';
        output_stream << "255\n";

        std::uint8_t const texel_size_in_bytes = subresource_layout.rowPitch / subresource_extent.width;

        for (std::uint32_t row_index = 0; row_index < subresource_extent.height; ++row_index)
        {
            for (std::uint32_t column_index = 0; column_index < subresource_extent.width; ++column_index)
            {
                std::uint64_t const texel_data_offset =
                    row_index * subresource_layout.rowPitch + texel_size_in_bytes * column_index;
                std::byte const* const texel_data = data_to_write.data() + texel_data_offset;

                std::array<char8_t, 4> color = {};
                std::memcpy(color.data(), texel_data, sizeof(decltype(color)::value_type) * color.size());

                output_stream << color[0] << ' ';
                output_stream << color[1] << ' ';
                output_stream << color[2] << "  ";
            }
            output_stream << '\n';
        }
    }
}