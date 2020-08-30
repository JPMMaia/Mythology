module maia.renderer.vulkan.serializer;

import maia.renderer.vulkan.check;

import <nlohmann/json.hpp>;
import <vulkan/vulkan.h>;

import <cstddef>;
import <filesystem>;
import <fstream>;
import <memory_resource>;
import <optional>;
import <span>;
import <string>;
import <vector>;

namespace Maia::Renderer::Vulkan
{
    namespace
    {
        std::pmr::vector<VkAttachmentDescription> create_attachments(
            nlohmann::json const& attachments_json,
            std::pmr::polymorphic_allocator<> const& output_allocator
        ) noexcept
        {
            auto const create_attachment = [] (nlohmann::json const& attachment_json) -> VkAttachmentDescription
            {
                return
                {
                    .flags = attachment_json.at("flags").get<VkAttachmentDescriptionFlags>(),
                    .format = attachment_json.at("format").get<VkFormat>(),
                    .samples = attachment_json.at("samples").get<VkSampleCountFlagBits>(),
                    .loadOp = attachment_json.at("load_operation").get<VkAttachmentLoadOp>(),
                    .storeOp = attachment_json.at("store_operation").get<VkAttachmentStoreOp>(),
                    .stencilLoadOp = attachment_json.at("stencil_load_operation").get<VkAttachmentLoadOp>(),
                    .stencilStoreOp = attachment_json.at("stencil_store_operation").get<VkAttachmentStoreOp>(),
                    .initialLayout = attachment_json.at("initial_layout").get<VkImageLayout>(),
                    .finalLayout = attachment_json.at("final_layout").get<VkImageLayout>(),
                };
            };

            std::pmr::vector<VkAttachmentDescription> attachments{output_allocator};
            attachments.resize(attachments_json.size());

            std::transform(attachments_json.begin(), attachments_json.end(), attachments.begin(), create_attachment);

            return attachments;
        }

        VkAttachmentReference create_attachment_reference(
            nlohmann::json const& attachment_reference_json
        ) noexcept
        {
            return
            {
                .attachment = attachment_reference_json.at("attachment").get<uint32_t>(),
                .layout = attachment_reference_json.at("layout").get<VkImageLayout>(),
            };
        };

        std::pmr::vector<VkAttachmentReference> create_attachment_references(
            nlohmann::json const& attachment_references_json,
            std::pmr::polymorphic_allocator<> const& output_allocator
        ) noexcept
        {
            std::pmr::vector<VkAttachmentReference> attachment_references{output_allocator};
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

        std::pmr::vector<VkAttachmentReference> create_subpasses_attachment_references(
            nlohmann::json const& subpasses_json,
            std::pmr::polymorphic_allocator<> const& output_allocator,
            std::pmr::polymorphic_allocator<> const& temporaries_allocator
        ) noexcept
        {
            std::pmr::vector<VkAttachmentReference> attachment_references{output_allocator};
            attachment_references.reserve(get_attachment_reference_count(subpasses_json));

            for (nlohmann::json const& subpass_json : subpasses_json)
            {
                assert(subpass_json.at("resolve_attachments").empty() || (subpass_json.at("resolve_attachments").size() == subpass_json.at("color_attachments").size()));

                std::pmr::vector<VkAttachmentReference> const input_attachments = 
                    create_attachment_references(subpass_json.at("input_attachments"), temporaries_allocator);
                attachment_references.insert(attachment_references.end(), input_attachments.begin(), input_attachments.end());

                std::pmr::vector<VkAttachmentReference> const color_attachments = 
                    create_attachment_references(subpass_json.at("color_attachments"), temporaries_allocator);
                attachment_references.insert(attachment_references.end(), color_attachments.begin(), color_attachments.end());

                std::pmr::vector<VkAttachmentReference> const resolve_attachments =
                    create_attachment_references(subpass_json.at("resolve_attachments"), temporaries_allocator);
                attachment_references.insert(attachment_references.end(), resolve_attachments.begin(), resolve_attachments.end());
                
                if (!subpass_json.at("depth_stencil_attachment").empty())
                {
                    VkAttachmentReference const depth_stencil_attachment = 
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
            std::pmr::vector<std::uint32_t> preserve_attachments{output_allocator};

            for (nlohmann::json const& subpass_json : subpasses_json)
            {
                for (nlohmann::json const& number_json : subpass_json.at("preserve_attachments"))
                {
                    preserve_attachments.push_back(number_json.get<std::uint32_t>());
                }
            }

            return preserve_attachments;
        }

        std::pmr::vector<VkSubpassDescription> create_subpasses(
            nlohmann::json const& subpasses_json,
            std::span<VkAttachmentReference const> const attachment_references,
            std::span<std::uint32_t const> const preserve_attachments,
            std::pmr::polymorphic_allocator<VkSubpassDescription> const& output_allocator
        ) noexcept
        {
            std::pmr::vector<VkSubpassDescription> subpasses{output_allocator};
            subpasses.reserve(subpasses_json.size());

            VkAttachmentReference const* current_attachment_reference = attachment_references.data();
            std::uint32_t const* current_preserve_attachment = preserve_attachments.data();

            for (nlohmann::json const& subpass_json : subpasses_json)
            {
                assert(subpass_json.at("resolve_attachments").empty() || (subpass_json.at("resolve_attachments").size() == subpass_json.at("color_attachments").size()));

                nlohmann::json const& input_attachments_json = subpass_json.at("input_attachments");
                nlohmann::json const& color_attachments_json = subpass_json.at("color_attachments");
                nlohmann::json const& resolve_attachments_json = subpass_json.at("resolve_attachments");
                nlohmann::json const& depth_stencil_attachment_json = subpass_json.at("depth_stencil_attachment");
                nlohmann::json const& preserve_attachment_json = subpass_json.at("preserve_attachments");

                VkAttachmentReference const* const input_attachments_pointer =
                    !input_attachments_json.empty() ? current_attachment_reference : nullptr;
                current_attachment_reference += input_attachments_json.size();

                VkAttachmentReference const* const color_attachments_pointer =
                    !color_attachments_json.empty() ? current_attachment_reference : nullptr;
                current_attachment_reference += color_attachments_json.size();

                VkAttachmentReference const* const resolve_attachment_pointer =
                    !resolve_attachments_json.empty() ? current_attachment_reference : nullptr;
                current_attachment_reference += resolve_attachments_json.size();

                VkAttachmentReference const* const depth_stencil_attachment_pointer =
                    !depth_stencil_attachment_json.empty() ? current_attachment_reference : nullptr;
                current_attachment_reference += (!depth_stencil_attachment_json.empty() ? 1 : 0);

                std::uint32_t const* const preserve_attachments_pointer =
                    !preserve_attachment_json.empty() ? current_preserve_attachment : nullptr;
                current_preserve_attachment += preserve_attachment_json.size();

                subpasses.push_back(
                    {
                        .flags = {},
                        .pipelineBindPoint = subpass_json.at("pipeline_bind_point").get<VkPipelineBindPoint>(),
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

        std::pmr::vector<VkSubpassDependency> create_dependencies(
            nlohmann::json const& dependencies_json,
            std::pmr::polymorphic_allocator<VkSubpassDependency> const& output_allocator
        ) noexcept
        {
            auto const create_dependency = [](nlohmann::json const& dependency_json) -> VkSubpassDependency
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
                    .srcStageMask = dependency_json.at("source_stage_mask").get<VkPipelineStageFlags>(),
                    .dstStageMask = dependency_json.at("destination_stage_mask").get<VkPipelineStageFlags>(),
                    .srcAccessMask = dependency_json.at("source_access_mask").get<VkAccessFlags>(),
                    .dstAccessMask = dependency_json.at("destination_access_mask").get<VkAccessFlags>(),
                    .dependencyFlags = dependency_json.at("dependency_flags").get<VkDependencyFlags>(),
                };
            };

            std::pmr::vector<VkSubpassDependency> dependencies{output_allocator};
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
        std::pmr::vector<VkAttachmentDescription> attachments = create_attachments(render_pass_json.at("attachments"), output_allocator);
        std::pmr::vector<VkAttachmentReference> attachment_references = create_subpasses_attachment_references(render_pass_json.at("subpasses"), output_allocator, temporaries_allocator);
        std::pmr::vector<std::uint32_t> preserve_attachments = create_subpasses_preserve_attachments(render_pass_json.at("subpasses"), output_allocator);
        std::pmr::vector<VkSubpassDescription> subpasses = create_subpasses(render_pass_json.at("subpasses"), attachment_references, preserve_attachments, output_allocator);
        std::pmr::vector<VkSubpassDependency> dependencies = create_dependencies(render_pass_json.at("dependencies"), output_allocator);
        
        VkRenderPassCreateInfo const create_info
        {
            .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
            .pNext = nullptr,
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

    std::pmr::vector<VkRenderPass> create_render_passes(
        VkDevice const device,
        VkAllocationCallbacks const* const allocation_callbacks,
        nlohmann::json const& render_passes_json,
        std::pmr::polymorphic_allocator<> const& output_allocator,
        std::pmr::polymorphic_allocator<> const& temporaries_allocator
    ) noexcept
    {
        std::pmr::vector<VkRenderPass> render_passes{output_allocator};
        render_passes.reserve(render_passes_json.size());

        for (nlohmann::json const& render_pass_json : render_passes_json)       
        {
            Render_pass_create_info_resources const create_info_resources = create_render_pass_create_info_resources(
                render_pass_json,
                temporaries_allocator,
                temporaries_allocator
            );

            VkRenderPass render_pass = {};
            check_result(
                vkCreateRenderPass(
                    device, 
                    &create_info_resources.create_info,
                    allocation_callbacks,
                    &render_pass
                )
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
            std::ifstream input_stream{file_path, std::ios::in | std::ios::binary};
            assert(input_stream.good());

            input_stream.seekg(0, std::ios::end);
            auto const size_in_bytes = input_stream.tellg();

            std::pmr::vector<std::byte> buffer{output_allocator};
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

            std::pmr::vector<Value_type> values{output_allocator};
            values.resize(bytes.size_bytes() / sizeof(Value_type));

            std::memcpy(values.data(), bytes.data(), bytes.size_bytes());

            return values;
        }
    }

    std::pmr::vector<VkShaderModule> create_shader_modules(
        VkDevice const device,
        VkAllocationCallbacks const* const allocation_callbacks,
        nlohmann::json const& shader_modules_json,
        std::filesystem::path const& shaders_path,
        std::pmr::polymorphic_allocator<> const& output_allocator,
        std::pmr::polymorphic_allocator<> const& temporaries_allocator
    ) noexcept
    {
        std::pmr::vector<VkShaderModule> shader_modules{output_allocator};
        shader_modules.reserve(shader_modules_json.size());

        for (nlohmann::json const& shader_module_json : shader_modules_json)       
        {
            std::string const& shader_file = shader_module_json.at("file").get<std::string>();

            std::pmr::vector<std::uint32_t> const shader_code = convert_bytes<std::uint32_t>(read_bytes(shaders_path / shader_file, temporaries_allocator), temporaries_allocator);

            VkShaderModuleCreateInfo const create_info
            {
                .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
                .pNext = nullptr,
                .flags = {},
                .codeSize = shader_code.size() * sizeof(decltype(shader_code)::value_type),
                .pCode = shader_code.data(),
            };

            VkShaderModule shader_module = {};
            check_result(
                vkCreateShaderModule(
                    device,
                    &create_info,
                    allocation_callbacks,
                    &shader_module
                )
            );

            shader_modules.push_back(shader_module);
        }

        return shader_modules;
    }

    std::pmr::vector<VkSampler> create_samplers(
        VkDevice const device,
        VkAllocationCallbacks const* const allocation_callbacks,
        nlohmann::json const& samplers_json,
        std::pmr::polymorphic_allocator<> const& output_allocator
    ) noexcept
    {
        std::pmr::vector<VkSampler> samplers{output_allocator};
        samplers.reserve(samplers_json.size());

        for (nlohmann::json const& sampler_json : samplers_json)       
        {
            VkSamplerCreateInfo const create_info
            {
                .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
                .pNext = nullptr,
                .flags = {},
                .magFilter = sampler_json.at("mag_filter").get<VkFilter>(),
                .minFilter = sampler_json.at("min_filter").get<VkFilter>(),
                .mipmapMode = sampler_json.at("mipmap_mode").get<VkSamplerMipmapMode>(),
                .addressModeU = sampler_json.at("address_mode_u").get<VkSamplerAddressMode>(),
                .addressModeV = sampler_json.at("address_mode_v").get<VkSamplerAddressMode>(),
                .addressModeW = sampler_json.at("address_mode_w").get<VkSamplerAddressMode>(),
                .mipLodBias = sampler_json.at("mip_lod_bias").get<float>(),
                .anisotropyEnable = sampler_json.at("anisotropy_enable").get<VkBool32>(),
                .maxAnisotropy = sampler_json.at("max_anisotropy").get<float>(),
                .compareEnable = sampler_json.at("compare_enable").get<VkBool32>(),
                .compareOp = sampler_json.at("compare_operation").get<VkCompareOp>(),
                .minLod = sampler_json.at("min_lod").get<float>(),
                .maxLod = sampler_json.at("max_lod").get<float>(),
                .borderColor = sampler_json.at("border_color").get<VkBorderColor>(),
                .unnormalizedCoordinates = sampler_json.at("unnormalized_coordinates").get<VkBool32>(),
            };

            VkSampler sampler = {};
            check_result(
                vkCreateSampler(
                    device,
                    &create_info,
                    allocation_callbacks,
                    &sampler
                )
            );

            samplers.push_back(sampler);
        }

        return samplers;
    }

    namespace
    {
        std::pmr::vector<VkSampler> arrange_immutable_samplers(
            nlohmann::json const& descriptor_set_layouts_json,
            std::span<VkSampler const> const samplers,
            std::pmr::polymorphic_allocator<> const& output_allocator
        ) noexcept
        {
            std::pmr::vector<VkSampler> immutable_samplers_per_descriptor_set_binding{output_allocator};

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

        std::pmr::vector<VkDescriptorSetLayoutBinding> create_descriptor_set_layouts_bindings(
            nlohmann::json const& descriptor_set_layouts_json,
            std::span<VkSampler const> const immutable_samplers_per_descriptor_set_binding,
            std::pmr::polymorphic_allocator<> const& output_allocator
        ) noexcept
        {
            std::pmr::vector<VkDescriptorSetLayoutBinding> bindings{output_allocator};

            std::uint32_t start_sampler_index = 0;

            for (nlohmann::json const& descriptor_set_layout_json : descriptor_set_layouts_json)       
            {
                for (nlohmann::json const& binding_json : descriptor_set_layout_json.at("bindings"))       
                {
                    nlohmann::json const& immutable_samplers_json = binding_json.at("immutable_samplers");

                    VkDescriptorSetLayoutBinding const binding
                    {
                        .binding = binding_json.at("binding").get<std::uint32_t>(),
                        .descriptorType = binding_json.at("descriptor_type").get<VkDescriptorType>(),
                        .descriptorCount = binding_json.at("descriptor_count").get<std::uint32_t>(),
                        .stageFlags = binding_json.at("stage_flags_property").get<VkShaderStageFlags>(),
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

    std::pmr::vector<VkDescriptorSetLayout> create_descriptor_set_layouts(
        VkDevice const device,
        VkAllocationCallbacks const* const allocation_callbacks,
        std::span<VkSampler const> const samplers,
        nlohmann::json const& descriptor_set_layouts_json,
        std::pmr::polymorphic_allocator<> const& output_allocator,
        std::pmr::polymorphic_allocator<> const& temporaries_allocator
    ) noexcept
    {
        std::pmr::vector<VkSampler> const immutable_samplers_per_descriptor_set_binding =
            arrange_immutable_samplers(descriptor_set_layouts_json, samplers, temporaries_allocator);

        std::pmr::vector<VkDescriptorSetLayoutBinding> const bindings = 
            create_descriptor_set_layouts_bindings(descriptor_set_layouts_json, immutable_samplers_per_descriptor_set_binding, temporaries_allocator);

        std::pmr::vector<VkDescriptorSetLayout> descriptor_set_layouts{output_allocator};
        descriptor_set_layouts.reserve(descriptor_set_layouts_json.size());

        std::uint32_t start_binding_index = 0;

        for (nlohmann::json const& descriptor_set_layout_json : descriptor_set_layouts_json)       
        {
            std::uint32_t const num_bindings = static_cast<std::uint32_t>(descriptor_set_layout_json.at("bindings").size());

            VkDescriptorSetLayoutCreateInfo const create_info
            {
                .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
                .pNext = nullptr,
                .flags = {},
                .bindingCount = num_bindings,
                .pBindings = bindings.data() + start_binding_index,
            };

            VkDescriptorSetLayout descriptor_set_layout = {};
            check_result(
                vkCreateDescriptorSetLayout(
                    device,
                    &create_info,
                    allocation_callbacks,
                    &descriptor_set_layout
                )
            );

            descriptor_set_layouts.push_back(descriptor_set_layout);
            start_binding_index += num_bindings;
        }

        return descriptor_set_layouts;
    }

    namespace
    {
        std::pmr::vector<VkDescriptorSetLayout> arrange_descriptor_set_layouts(
            nlohmann::json const& pipeline_layouts_json,
            std::span<VkDescriptorSetLayout const> const descriptor_set_layouts,
            std::pmr::polymorphic_allocator<> const& output_allocator
        ) noexcept
        {
            std::pmr::vector<VkDescriptorSetLayout> descriptor_set_layouts_per_pipeline_layout{output_allocator};

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

        std::pmr::vector<VkPushConstantRange> create_push_constant_ranges(
            nlohmann::json const& pipeline_layouts_json,
            std::pmr::polymorphic_allocator<> const& output_allocator
        ) noexcept
        {
            auto const create_push_constant_range = [](nlohmann::json const& push_constant_range_json) -> VkPushConstantRange
            {
                return
                {
                    .stageFlags = push_constant_range_json.at("stage_flags").get<VkShaderStageFlags>(),
                    .offset = push_constant_range_json.at("offset").get<std::uint32_t>(),
                    .size = push_constant_range_json.at("size").get<std::uint32_t>(),
                };
            };

            std::pmr::vector<VkPushConstantRange> push_constant_ranges{output_allocator};

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

    std::pmr::vector<VkPipelineLayout> create_pipeline_layouts(
        VkDevice const device,
        VkAllocationCallbacks const* const allocation_callbacks,
        std::span<VkDescriptorSetLayout const> const descriptor_set_layouts,
        nlohmann::json const& pipeline_layouts_json,
        std::pmr::polymorphic_allocator<> const& output_allocator,
        std::pmr::polymorphic_allocator<> const& temporaries_allocator
    ) noexcept
    {
        std::pmr::vector<VkDescriptorSetLayout> const descriptor_set_layouts_per_pipeline_layout =
            arrange_descriptor_set_layouts(pipeline_layouts_json, descriptor_set_layouts, temporaries_allocator);

        std::pmr::vector<VkPushConstantRange> const push_constant_ranges =
            create_push_constant_ranges(pipeline_layouts_json, temporaries_allocator);

        std::pmr::vector<VkPipelineLayout> pipeline_layouts{output_allocator};
        pipeline_layouts.reserve(pipeline_layouts_json.size());

        std::uint32_t start_descriptor_set_layout_index = 0;
        std::uint32_t start_push_constant_range_index = 0;

        for (nlohmann::json const& pipeline_layout_json : pipeline_layouts_json)       
        {
            std::uint32_t const num_descriptor_set_layouts = static_cast<std::uint32_t>(pipeline_layout_json.at("descriptor_set_layouts").size());
            std::uint32_t const num_push_constant_ranges = static_cast<std::uint32_t>(pipeline_layout_json.at("push_constant_ranges").size());

            VkPipelineLayoutCreateInfo const create_info
            {
                .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
                .pNext = nullptr,
                .flags = {},
                .setLayoutCount = num_descriptor_set_layouts,
                .pSetLayouts = descriptor_set_layouts_per_pipeline_layout.data() + start_descriptor_set_layout_index,
                .pushConstantRangeCount = num_push_constant_ranges,
                .pPushConstantRanges = push_constant_ranges.data() + start_push_constant_range_index,
            };

            VkPipelineLayout pipeline_layout = {};
            check_result(
                vkCreatePipelineLayout(
                    device,
                    &create_info,
                    allocation_callbacks,
                    &pipeline_layout
                )
            );

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
            std::pmr::vector<std::pmr::string> stage_names{output_allocator};

            for (nlohmann::json const& pipeline_state_json : pipeline_states_json)
            {
                for (nlohmann::json const& stage_json : pipeline_state_json.at("stages"))
                {
                    std::string const& name = stage_json.at("entry_point").get<std::string>();

                    stage_names.push_back(
                        std::pmr::string{name, output_allocator}
                    );
                }
            }

            return stage_names;
        }

        VkPipelineShaderStageCreateInfo create_pipeline_shader_stage_create_info(
            nlohmann::json const& json,
            std::span<VkShaderModule const> const shader_modules,
            char const* const name
        ) noexcept
        {
            return
            {
                .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
                .pNext = nullptr,
                .flags = {},
                .stage = json.at("stage").get<VkShaderStageFlagBits>(),
                .module = shader_modules[json.at("shader").get<std::size_t>()],
                .pName = name,
                .pSpecializationInfo = nullptr,
            };
        }

        std::pmr::vector<VkPipelineShaderStageCreateInfo> create_pipeline_shader_stage_create_infos(
            nlohmann::json const& pipeline_states_json,
            std::span<VkShaderModule const> const shader_modules,
            std::span<std::pmr::string const> const stage_names,
            std::pmr::polymorphic_allocator<> const& output_allocator
        ) noexcept
        {
            std::pmr::vector<VkPipelineShaderStageCreateInfo> create_infos{output_allocator};

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

        VkVertexInputBindingDescription create_vertex_input_binding_description(
            nlohmann::json const& json
        ) noexcept
        {
            return 
            {
                .binding = json.at("binding").get<std::uint32_t>(),
                .stride = json.at("stride").get<std::uint32_t>(),
                .inputRate = json.at("input_rate").get<VkVertexInputRate>(),
            };
        }

        std::pmr::vector<VkVertexInputBindingDescription> create_vertex_input_binding_descriptions(
            nlohmann::json const& pipeline_states_json,
            std::pmr::polymorphic_allocator<> const& output_allocator
        ) noexcept
        {
            std::pmr::vector<VkVertexInputBindingDescription> descriptions{output_allocator};

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

        VkVertexInputAttributeDescription create_vertex_input_attribute_description(
            nlohmann::json const& json
        ) noexcept
        {
            return 
            {
                .location = json.at("location").get<std::uint32_t>(),
                .binding = json.at("binding").get<std::uint32_t>(),
                .format = json.at("format").get<VkFormat>(),
                .offset = json.at("offset").get<std::uint32_t>(),
            };
        }

        std::pmr::vector<VkVertexInputAttributeDescription> create_vertex_input_attribute_descriptions(
            nlohmann::json const& pipeline_states_json,
            std::pmr::polymorphic_allocator<> const& output_allocator
        ) noexcept
        {
            std::pmr::vector<VkVertexInputAttributeDescription> descriptions{output_allocator};

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


        VkPipelineVertexInputStateCreateInfo create_pipeline_vertex_input_state_create_info(
            nlohmann::json const& json,
            std::span<VkVertexInputBindingDescription const> const bindings,
            std::span<VkVertexInputAttributeDescription const> const attributes
        ) noexcept
        {
            return 
            {
                .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
                .pNext = nullptr,
                .flags = {},
                .vertexBindingDescriptionCount = static_cast<std::uint32_t>(bindings.size()),
                .pVertexBindingDescriptions = bindings.data(),
                .vertexAttributeDescriptionCount = static_cast<std::uint32_t>(attributes.size()),
                .pVertexAttributeDescriptions = attributes.data(),
            };
        }

        std::pmr::vector<VkPipelineVertexInputStateCreateInfo> create_pipeline_vertex_input_state_create_infos(
            nlohmann::json const& pipeline_states_json,
            std::span<VkVertexInputBindingDescription const> const bindings,
            std::span<VkVertexInputAttributeDescription const> const attributes,
            std::pmr::polymorphic_allocator<> const& output_allocator
        ) noexcept
        {
            std::pmr::vector<VkPipelineVertexInputStateCreateInfo> create_infos{output_allocator};
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
                        {bindings.data() + start_binding_index, binding_count},
                        {attributes.data() + start_attribute_index, start_attribute_index}
                    )
                );

                start_binding_index += binding_count;
                start_attribute_index += attribute_count;
            }

            return create_infos;
        }


        VkPipelineInputAssemblyStateCreateInfo create_pipeline_input_assembly_state_create_info(
            nlohmann::json const& json
        ) noexcept
        {
            return
            {
                .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
                .pNext = nullptr,
                .flags = {},
                .topology = json.at("topology").get<VkPrimitiveTopology>(),
                .primitiveRestartEnable = json.at("primitive_restart_enable").get<VkBool32>(),
            };
        }

        std::pmr::vector<VkPipelineInputAssemblyStateCreateInfo> create_pipeline_input_assembly_state_create_infos(
            nlohmann::json const& pipeline_states_json,
            std::pmr::polymorphic_allocator<> const& output_allocator
        ) noexcept
        {
            std::pmr::vector<VkPipelineInputAssemblyStateCreateInfo> create_infos{output_allocator};

            for (nlohmann::json const& pipeline_state_json : pipeline_states_json)
            {
                create_infos.push_back(
                    create_pipeline_input_assembly_state_create_info(pipeline_state_json.at("input_assembly_state"))
                );
            }

            return create_infos;
        }


        VkPipelineTessellationStateCreateInfo create_pipeline_tessellation_state_create_info(
            nlohmann::json const& json
        ) noexcept
        {
            return {};
        }

        std::pmr::vector<std::optional<VkPipelineTessellationStateCreateInfo>> create_pipeline_tessellation_state_create_infos(
            nlohmann::json const& pipeline_states_json,
            std::pmr::polymorphic_allocator<> const& output_allocator
        ) noexcept
        {
            std::pmr::vector<std::optional<VkPipelineTessellationStateCreateInfo>> create_infos{output_allocator};
            create_infos.reserve(pipeline_states_json.size());

            for (nlohmann::json const& pipeline_state_json : pipeline_states_json)
            {
                create_infos.push_back(
                    {}
                );
            }

            return create_infos;
        }

        VkViewport create_viewport(
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

        std::pmr::vector<VkViewport> create_viewports(
            nlohmann::json const& pipeline_states_json,
            std::pmr::polymorphic_allocator<> const& output_allocator
        ) noexcept
        {
            std::pmr::vector<VkViewport> viewports{output_allocator};

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

        VkOffset2D create_offset_2d(
            nlohmann::json const& json
        ) noexcept
        {
            return
            {
                .x = json.at("x").get<std::int32_t>(),
                .y = json.at("y").get<std::int32_t>(),
            };
        }

        VkExtent2D create_extent_2d(
            nlohmann::json const& json
        ) noexcept
        {
            return
            {
                .width = json.at("width").get<std::uint32_t>(),
                .height = json.at("height").get<std::uint32_t>(),
            };
        }

        VkRect2D create_rect_2d(
            nlohmann::json const& json
        ) noexcept
        {
            return
            {
                .offset = create_offset_2d(json.at("offset")),
                .extent = create_extent_2d(json.at("extent")),
            };
        }

        std::pmr::vector<VkRect2D> create_scissors(
            nlohmann::json const& pipeline_states_json,
            std::pmr::polymorphic_allocator<> const& output_allocator
        ) noexcept
        {
            std::pmr::vector<VkRect2D> scissors{output_allocator};

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


        VkPipelineViewportStateCreateInfo create_pipeline_viewport_state_create_info(
            nlohmann::json const& json,
            std::span<VkViewport const> const viewports,
            std::span<VkRect2D const> const scissors
        ) noexcept
        {
            assert(json.at("viewports").size() == viewports.size());
            assert(json.at("scissors").size() == scissors.size());
            assert((json.at("viewport_count").get<std::uint32_t>() == json.at("viewports").size()) || json.at("viewports").is_null());
            assert((json.at("scissor_count").get<std::uint32_t>() == json.at("scissors").size()) || json.at("scissors").is_null());

            return
            {
                .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
                .pNext = nullptr,
                .flags = {},
                .viewportCount = json.at("viewport_count").get<std::uint32_t>(),
                .pViewports = !viewports.empty() ? viewports.data() : nullptr,
                .scissorCount = json.at("scissor_count").get<std::uint32_t>(),
                .pScissors = !scissors.empty() ? scissors.data() : nullptr,
            };
        }

        std::pmr::vector<VkPipelineViewportStateCreateInfo> create_pipeline_viewport_state_create_infos(
            nlohmann::json const& pipeline_states_json,
            std::span<VkViewport const> const viewports,
            std::span<VkRect2D const> const scissors,
            std::pmr::polymorphic_allocator<> const& output_allocator
        ) noexcept
        {
            std::pmr::vector<VkPipelineViewportStateCreateInfo> create_infos{output_allocator};
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
                        {viewports.data() + start_viewport_index, viewport_count},
                        {scissors.data() + start_scissor_index, scissor_count}
                    )
                );

                start_viewport_index += viewport_count;
                start_scissor_index += scissor_count;
            }

            return create_infos;
        }


        VkPipelineRasterizationStateCreateInfo create_pipeline_rasterization_state_create_info(
            nlohmann::json const& json
        ) noexcept
        {
            return
            {
                .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
                .pNext = nullptr,
                .flags = {},
                .depthClampEnable = json.at("depth_clamp_enable").get<VkBool32>(),
                .rasterizerDiscardEnable = json.at("rasterizer_discard_enable").get<VkBool32>(),
                .polygonMode = json.at("polygon_mode").get<VkPolygonMode>(),
                .cullMode = json.at("cull_mode").get<VkCullModeFlags>(),
                .frontFace = json.at("front_face").get<VkFrontFace>(),
                .depthBiasEnable = json.at("depth_bias_enable").get<VkBool32>(),
                .depthBiasConstantFactor = json.at("depth_bias_constant_factor").get<float>(),
                .depthBiasClamp = json.at("depth_bias_clamp").get<float>(),
                .depthBiasSlopeFactor = json.at("depth_bias_slope_factor").get<float>(),
                .lineWidth = json.at("line_width_factor").get<float>(),
            };
        }

        std::pmr::vector<VkPipelineRasterizationStateCreateInfo> create_pipeline_rasterization_state_create_infos(
            nlohmann::json const& pipeline_states_json,
            std::pmr::polymorphic_allocator<> const& output_allocator
        ) noexcept
        {
            std::pmr::vector<VkPipelineRasterizationStateCreateInfo> create_infos{output_allocator};

            for (nlohmann::json const& pipeline_state_json : pipeline_states_json)
            {
                create_infos.push_back(
                    create_pipeline_rasterization_state_create_info(pipeline_state_json.at("rasterization_state"))
                );
            }

            return create_infos;
        }


        VkPipelineMultisampleStateCreateInfo create_pipeline_multisample_state_create_info(
            nlohmann::json const& json
        ) noexcept
        {
            static std::uint32_t constexpr sample_mask = 0xFFFFFFFF;

            return
            {
                .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
                .pNext = nullptr,
                .flags = {},
                .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
                .sampleShadingEnable = VK_FALSE,
                .minSampleShading = {},
                .pSampleMask = &sample_mask,
                .alphaToCoverageEnable = VK_FALSE,
                .alphaToOneEnable = VK_FALSE,
            };
        }

        std::pmr::vector<VkPipelineMultisampleStateCreateInfo> create_pipeline_multisample_state_create_infos(
            nlohmann::json const& pipeline_states_json,
            std::pmr::polymorphic_allocator<> const& output_allocator
        ) noexcept
        {
            std::pmr::vector<VkPipelineMultisampleStateCreateInfo> create_infos{output_allocator};
            create_infos.reserve(pipeline_states_json.size());

            for (nlohmann::json const& pipeline_state_json : pipeline_states_json)
            {
                create_infos.push_back(
                    create_pipeline_multisample_state_create_info({})
                );
            }

            return create_infos;
        }

        VkStencilOpState create_stencil_operation_state(
            nlohmann::json const& json
        ) noexcept
        {
            return
            {
                .failOp = json.at("fail_operation").get<VkStencilOp>(),
                .passOp = json.at("pass_operation").get<VkStencilOp>(),
                .depthFailOp = json.at("depth_fail_operation").get<VkStencilOp>(),
                .compareOp = json.at("compare_operation").get<VkCompareOp>(),
                .compareMask = json.at("compare_mask").get<std::uint32_t>(),
                .writeMask = json.at("write_mask").get<std::uint32_t>(),
                .reference = json.at("reference").get<std::uint32_t>(),
            };
        }

        VkPipelineDepthStencilStateCreateInfo create_pipeline_depth_stencil_state_create_info(
            nlohmann::json const& json
        ) noexcept
        {
            return
            {
                .sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
                .pNext = nullptr,
                .flags = {},
                .depthTestEnable = json.at("depth_test_enable").get<VkBool32>(),
                .depthWriteEnable = json.at("depth_write_enable").get<VkBool32>(),
                .depthCompareOp = json.at("compare_operation").get<VkCompareOp>(),
                .depthBoundsTestEnable = json.at("depth_bounds_test_enable").get<VkBool32>(),
                .stencilTestEnable = json.at("stencil_test_enable").get<VkBool32>(),
                .front = create_stencil_operation_state(json.at("front_stencil_state")),
                .back = create_stencil_operation_state(json.at("back_stencil_state")),
                .minDepthBounds = json.at("min_depth_bounds").get<float>(),
                .maxDepthBounds = json.at("max_depth_bounds").get<float>(),
            };
        }

        std::pmr::vector<VkPipelineDepthStencilStateCreateInfo> create_pipeline_depth_stencil_state_create_infos(
            nlohmann::json const& pipeline_states_json,
            std::pmr::polymorphic_allocator<> const& output_allocator
        ) noexcept
        {
            std::pmr::vector<VkPipelineDepthStencilStateCreateInfo> create_infos{output_allocator};

            for (nlohmann::json const& pipeline_state_json : pipeline_states_json)
            {
                create_infos.push_back(
                    create_pipeline_depth_stencil_state_create_info(pipeline_state_json.at("depth_stencil_state"))
                );
            }

            return create_infos;
        }


        VkPipelineColorBlendAttachmentState create_pipeline_color_blend_attachment_state(
            nlohmann::json const& json
        ) noexcept
        {
            return
            {
                .blendEnable = json.at("blend_enable").get<VkBool32>(),
                .srcColorBlendFactor = json.at("source_color_blend_factor").get<VkBlendFactor>(),
                .dstColorBlendFactor = json.at("destination_color_blend_factor").get<VkBlendFactor>(),
                .colorBlendOp = json.at("color_blend_operation").get<VkBlendOp>(),
                .srcAlphaBlendFactor = json.at("source_alpha_blend_factor").get<VkBlendFactor>(),
                .dstAlphaBlendFactor = json.at("destination_alpha_blend_factor").get<VkBlendFactor>(),
                .alphaBlendOp = json.at("alpha_blend_operation").get<VkBlendOp>(),
                .colorWriteMask = json.at("color_write_mask").get<VkColorComponentFlags>(),
            };
        }

        std::pmr::vector<VkPipelineColorBlendAttachmentState> create_pipeline_color_blend_attachment_states(
            nlohmann::json const& pipeline_states_json,
            std::pmr::polymorphic_allocator<> const& output_allocator
        ) noexcept
        {
            std::pmr::vector<VkPipelineColorBlendAttachmentState> attachments{output_allocator};

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

        VkPipelineColorBlendStateCreateInfo create_pipeline_color_blend_state_create_info(
            nlohmann::json const& json,
            std::span<VkPipelineColorBlendAttachmentState const> const attachments
        ) noexcept
        {
            assert(attachments.size() == json.at("attachments").size());

            nlohmann::json const& blend_constants = json.at("blend_constants");
            assert(blend_constants.size() == 4);

            return
            {
                .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
                .pNext = nullptr,
                .flags = {},
                .logicOpEnable = json.at("logic_operation_enable").get<VkBool32>(),
                .logicOp = json.at("logic_operation").get<VkLogicOp>(),
                .attachmentCount = static_cast<std::uint32_t>(attachments.size()),
                .pAttachments = attachments.data(),
                .blendConstants = {
                    static_cast<float>(blend_constants[0]),
                    static_cast<float>(blend_constants[1]),
                    static_cast<float>(blend_constants[2]),
                    static_cast<float>(blend_constants[3])
                },
            };
        }

        std::pmr::vector<VkPipelineColorBlendStateCreateInfo> create_pipeline_color_blend_state_create_infos(
            nlohmann::json const& pipeline_states_json,
            std::span<VkPipelineColorBlendAttachmentState const> const attachments,
            std::pmr::polymorphic_allocator<> const& output_allocator
        ) noexcept
        {
            std::pmr::vector<VkPipelineColorBlendStateCreateInfo> create_infos{output_allocator};
            create_infos.reserve(pipeline_states_json.size());

            std::size_t start_attachment_index = 0;

            for (nlohmann::json const& pipeline_state_json : pipeline_states_json)
            {
                nlohmann::json const& color_blend_state_json = pipeline_state_json.at("color_blend_state");
                
                std::size_t const attachment_count = color_blend_state_json.at("attachments").size();

                create_infos.push_back(
                    create_pipeline_color_blend_state_create_info(
                        color_blend_state_json,
                        {attachments.data() + start_attachment_index, attachment_count}
                    )
                );
            }

            return create_infos;
        }


        std::pmr::vector<VkDynamicState> create_dynamic_states(
            nlohmann::json const& pipeline_states_json,
            std::pmr::polymorphic_allocator<> const& output_allocator
        ) noexcept
        {
            std::pmr::vector<VkDynamicState> dynamic_states{output_allocator};

            for (nlohmann::json const& pipeline_state_json : pipeline_states_json)
            {
                for (nlohmann::json const& dynamic_state_json : pipeline_state_json.at("dynamic_state").at("dynamic_states"))
                {
                    dynamic_states.push_back(
                        dynamic_state_json.get<VkDynamicState>()
                    );
                }
            }

            return dynamic_states;
        }

        VkPipelineDynamicStateCreateInfo create_pipeline_dynamic_state_create_info(
            nlohmann::json const& json,
            std::span<VkDynamicState const> const dynamic_states
        ) noexcept
        {
            assert(dynamic_states.size() == json.at("dynamic_states").size());

            return 
            {
                .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
                .pNext = nullptr,
                .flags = {},
                .dynamicStateCount = static_cast<std::uint32_t>(dynamic_states.size()),
                .pDynamicStates = dynamic_states.data(),
            };
        }

        std::pmr::vector<VkPipelineDynamicStateCreateInfo> create_pipeline_dynamic_state_create_infos(
            nlohmann::json const& pipeline_states_json,
            std::span<VkDynamicState const> const dynamic_states,
            std::pmr::polymorphic_allocator<> const& output_allocator
        ) noexcept
        {
            std::pmr::vector<VkPipelineDynamicStateCreateInfo> create_infos{output_allocator};
            create_infos.reserve(pipeline_states_json.size());

            std::size_t start_dynamic_state_index = 0;

            for (nlohmann::json const& pipeline_state_json : pipeline_states_json)
            {
                nlohmann::json const& pipeline_dynamic_state_json = pipeline_state_json.at("dynamic_state");

                std::size_t const dynamic_state_count = pipeline_dynamic_state_json.at("dynamic_states").size();

                create_infos.push_back(
                    create_pipeline_dynamic_state_create_info(
                        pipeline_dynamic_state_json,
                        {dynamic_states.data() + start_dynamic_state_index, dynamic_state_count}
                    )
                );

                start_dynamic_state_index += dynamic_state_count;
            }

            return create_infos;
        }

    }

    std::pmr::vector<VkPipeline> create_pipeline_states(
        VkDevice const device,
        VkAllocationCallbacks const* const allocation_callbacks,
        std::span<VkShaderModule const> const shader_modules,
        std::span<VkPipelineLayout const> const pipeline_layouts,
        std::span<VkRenderPass const> const render_passes,
        nlohmann::json const& pipeline_states_json,
        std::pmr::polymorphic_allocator<> const& output_allocator,
        std::pmr::polymorphic_allocator<> const& temporaries_allocator
    ) noexcept
    {
        std::pmr::vector<std::pmr::string> const shader_stage_names = 
            create_pipeline_shader_stage_names(
                pipeline_states_json,
                temporaries_allocator
            );

        std::pmr::vector<VkPipelineShaderStageCreateInfo> const shader_stages = 
            create_pipeline_shader_stage_create_infos(
                pipeline_states_json,
                shader_modules,
                shader_stage_names,
                temporaries_allocator
        );

        std::pmr::vector<VkVertexInputBindingDescription> const vertex_input_binding_descriptions = 
            create_vertex_input_binding_descriptions(
                pipeline_states_json,
                temporaries_allocator
            );

        std::pmr::vector<VkVertexInputAttributeDescription> const vertex_input_attribute_descriptions = 
            create_vertex_input_attribute_descriptions(
                pipeline_states_json,
                temporaries_allocator
            );

        std::pmr::vector<VkPipelineVertexInputStateCreateInfo> const vertex_input_states = 
            create_pipeline_vertex_input_state_create_infos(
                pipeline_states_json,
                vertex_input_binding_descriptions,
                vertex_input_attribute_descriptions,
                temporaries_allocator
            );

        std::pmr::vector<VkPipelineInputAssemblyStateCreateInfo> const input_assembly_states = 
            create_pipeline_input_assembly_state_create_infos(
                pipeline_states_json,
                temporaries_allocator
            );

        std::pmr::vector<std::optional<VkPipelineTessellationStateCreateInfo>> const tessellation_states = 
            create_pipeline_tessellation_state_create_infos(
                pipeline_states_json,
                temporaries_allocator
            );

        std::pmr::vector<VkViewport> const viewports = create_viewports(pipeline_states_json, temporaries_allocator);

        std::pmr::vector<VkRect2D> const scissors = create_scissors(pipeline_states_json, temporaries_allocator);

        std::pmr::vector<VkPipelineViewportStateCreateInfo> const viewport_states = 
            create_pipeline_viewport_state_create_infos(
                pipeline_states_json,
                viewports,
                scissors,
                temporaries_allocator
            );

        std::pmr::vector<VkPipelineRasterizationStateCreateInfo> const rasterization_states = 
            create_pipeline_rasterization_state_create_infos(
                pipeline_states_json,
                temporaries_allocator
            );

        std::pmr::vector<VkPipelineMultisampleStateCreateInfo> const multisample_states = 
            create_pipeline_multisample_state_create_infos(
                pipeline_states_json,
                temporaries_allocator
            );

        std::pmr::vector<VkPipelineDepthStencilStateCreateInfo> const depth_stencil_states = 
            create_pipeline_depth_stencil_state_create_infos(
                pipeline_states_json,
                temporaries_allocator
            );

        std::pmr::vector<VkPipelineColorBlendAttachmentState> const color_blend_attachment_states = 
            create_pipeline_color_blend_attachment_states(
                pipeline_states_json,
                temporaries_allocator
            );

        std::pmr::vector<VkPipelineColorBlendStateCreateInfo> const color_blend_states = 
            create_pipeline_color_blend_state_create_infos(
                pipeline_states_json,
                color_blend_attachment_states,
                temporaries_allocator
            );

        std::pmr::vector<VkDynamicState> const dynamic_states = create_dynamic_states(pipeline_states_json, temporaries_allocator);

        std::pmr::vector<VkPipelineDynamicStateCreateInfo> const pipeline_dynamic_states = 
            create_pipeline_dynamic_state_create_infos(
                pipeline_states_json,
                dynamic_states,
                temporaries_allocator
            );


        std::pmr::vector<VkGraphicsPipelineCreateInfo> create_infos{temporaries_allocator};
        create_infos.reserve(pipeline_states_json.size());

        {
            std::size_t pipeline_state_index = 0;
            std::size_t start_stage_index = 0; 

            for (nlohmann::json const& pipeline_state_json : pipeline_states_json)
            {
                std::size_t const stage_count = pipeline_state_json.at("stages").size();

                VkGraphicsPipelineCreateInfo const create_info
                {
                    .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
                    .pNext = nullptr,
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
            std::pmr::vector<VkPipeline> pipelines{output_allocator};
            pipelines.resize(pipeline_states_json.size());

            assert(create_infos.size() == pipelines.size());
            check_result(
                vkCreateGraphicsPipelines(
                    device,
                    VK_NULL_HANDLE,
                    static_cast<std::uint32_t>(create_infos.size()),
                    create_infos.data(),
                    allocation_callbacks,
                    pipelines.data()
                )
            );

            return pipelines;
        }
        else
        {
            return {};
        }
    }

    namespace
    {
        enum class Command_type : std::uint8_t
        {
            Begin_render_pass,
            Bind_pipeline,
            Clear_color_image,
            Draw,
            End_render_pass,
            Pipeline_barrier,
            Set_screen_viewport_and_scissors
        };

        namespace Begin_render_pass
        {
            enum class Type : std::uint8_t
            {
                Dependent
            };

            struct Dependent
            {
                VkRenderPass render_pass;
            };
        }

        std::pmr::vector<std::byte> create_begin_render_pass_data(
            nlohmann::json const& json,
            std::span<VkRenderPass const> const render_passes,
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

            std::pmr::vector<std::byte> data{output_allocator};
            data.resize(sizeof(command_type) + sizeof(begin_render_pass_type) + sizeof(dependent));
            std::memcpy(data.data(), &command_type, sizeof(command_type));
            std::memcpy(data.data() + sizeof(command_type), &begin_render_pass_type, sizeof(begin_render_pass_type));
            std::memcpy(data.data() + sizeof(command_type) + sizeof(begin_render_pass_type), &dependent, sizeof(dependent));
            return data;
        }

        struct Bind_pipeline
        {
            VkPipelineBindPoint bind_point;
            VkPipeline pipeline;
        };

        std::pmr::vector<std::byte> create_bind_pipeline_data(
            nlohmann::json const& json,
            std::span<VkPipeline const> const pipelines,
            std::pmr::polymorphic_allocator<> const& output_allocator
        ) noexcept
        {
            assert(json.at("type").get<std::string>() == "Bind_pipeline");

            Command_type constexpr command_type = Command_type::Bind_pipeline;

            Bind_pipeline const command
            {
                .bind_point = json.at("pipeline_bind_point").get<VkPipelineBindPoint>(),
                .pipeline = pipelines[json.at("pipeline").get<std::size_t>()]
            };

            std::pmr::vector<std::byte> data{output_allocator};
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
                VkClearColorValue clear_color_value;
            };
        }

        VkClearColorValue create_color_color_value(
            nlohmann::json const& clear_color_value_json
        ) noexcept
        {
            std::string const& type = clear_color_value_json.at("type").get<std::string>();
            assert(type == "INT" || type == "UINT" || type == "FLOAT");

            nlohmann::json const& values_json = clear_color_value_json.at("values");

            if (type == "FLOAT")
            {
                return
                {
                    .float32 = {values_json[0].get<float>(), values_json[1].get<float>(), values_json[2].get<float>(), values_json[3].get<float>()}
                };
            }
            else if (type == "INT")
            {
                return
                {
                    .int32 = {values_json[0].get<std::int32_t>(), values_json[1].get<std::int32_t>(), values_json[2].get<std::int32_t>(), values_json[3].get<std::int32_t>()}
                };
            }
            else
            {
                assert(type == "UINT");

                return
                {
                    .uint32 = {values_json[0].get<std::uint32_t>(), values_json[1].get<std::uint32_t>(), values_json[2].get<std::uint32_t>(), values_json[3].get<std::uint32_t>()}
                };
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

            std::pmr::vector<std::byte> data{output_allocator};
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

            std::pmr::vector<std::byte> data{output_allocator};
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

            std::pmr::vector<std::byte> data{output_allocator};
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
                VkAccessFlagBits source_access_mask;
                VkAccessFlagBits destination_access_mask;
                VkImageLayout old_layout;
                VkImageLayout new_layout;
            };
        }

        struct Pipeline_barrier
        {
            VkPipelineStageFlagBits source_stage_mask;
            VkPipelineStageFlagBits destination_stage_mask;
            VkDependencyFlagBits dependency_flags;
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
                .source_access_mask = image_barrier_json.at("source_access_mask").get<VkAccessFlagBits>(),
                .destination_access_mask = image_barrier_json.at("destination_access_mask").get<VkAccessFlagBits>(),
                .old_layout = image_barrier_json.at("old_layout").get<VkImageLayout>(),
                .new_layout = image_barrier_json.at("new_layout").get<VkImageLayout>(),
            };

            Image_memory_barrier::Type constexpr barrier_type = Image_memory_barrier::Type::Dependent;

            std::pmr::vector<std::byte> data{output_allocator};
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
                .source_stage_mask = command_json.at("source_stage_mask").get<VkPipelineStageFlagBits>(),
                .destination_stage_mask = command_json.at("destination_stage_mask").get<VkPipelineStageFlagBits>(),
                .dependency_flags = command_json.at("dependency_flags").get<VkDependencyFlagBits>(),
                .memory_barrier_count = static_cast<std::uint8_t>(command_json.at("memory_barriers").size()),
                .buffer_barrier_count = static_cast<std::uint8_t>(command_json.at("buffer_barriers").size()),
                .image_barrier_count = static_cast<std::uint8_t>(command_json.at("image_barriers").size())
            };

            Command_type constexpr command_type = Command_type::Pipeline_barrier;

            std::pmr::vector<std::byte> data{output_allocator};
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

            std::pmr::vector<std::byte> data{output_allocator};
            data.resize(sizeof(command_type));
            std::memcpy(data.data(), &command_type, sizeof(command_type));
            return data;
        }


        std::pmr::vector<std::byte> create_command_data(
            nlohmann::json const& command_json,
            std::span<VkPipeline const> const pipelines,
            std::span<VkRenderPass const> const render_passes,
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
        std::span<VkPipeline const> const pipelines,
        std::span<VkRenderPass const> const render_passes,
        std::pmr::polymorphic_allocator<std::byte> const& output_allocator,
        std::pmr::polymorphic_allocator<std::byte> const& temporaries_allocator
    ) noexcept
    {
        std::pmr::vector<std::byte> commands_data{output_allocator};

        for (nlohmann::json const& draw_list_json : commands_json)
        {
            for (nlohmann::json const& command_json : draw_list_json)
            {
                std::pmr::vector<std::byte> const command_data = create_command_data(command_json, pipelines, render_passes, temporaries_allocator, temporaries_allocator);

                commands_data.insert(commands_data.end(), command_data.begin(), command_data.end());
            }
        }

        return {commands_data};
    }

    namespace
    {
        using Commands_data_offset = size_t;

        Commands_data_offset add_begin_render_pass_command(
            VkCommandBuffer const command_buffer,
            VkFramebuffer const framebuffer,
            VkRect2D const framebuffer_render_area,
            std::span<std::byte const> const bytes
        ) noexcept
        {
            Commands_data_offset commands_data_offset = 0;

            Begin_render_pass::Type const subtype = read<Begin_render_pass::Type>(bytes.data() + commands_data_offset);
            commands_data_offset += sizeof(subtype);

            assert(subtype == Begin_render_pass::Type::Dependent);

            Begin_render_pass::Dependent const command = read<Begin_render_pass::Dependent>(bytes.data() + commands_data_offset);
            commands_data_offset += sizeof(command);

            VkRenderPassBeginInfo const begin_info
            {
                .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
                .pNext = nullptr,
                .renderPass = command.render_pass,
                .framebuffer = framebuffer,
                .renderArea = framebuffer_render_area,
                .clearValueCount = 0,
                .pClearValues = nullptr,
            };

            vkCmdBeginRenderPass(
                command_buffer,
                &begin_info,
                VK_SUBPASS_CONTENTS_INLINE
            );

            return commands_data_offset;
        }

        Commands_data_offset add_bind_pipeline_command(
            VkCommandBuffer const command_buffer,
            std::span<std::byte const> const bytes
        ) noexcept
        {
            Commands_data_offset commands_data_offset = 0;

            Bind_pipeline const command = read<Bind_pipeline>(bytes.data() + commands_data_offset);
            commands_data_offset += sizeof(command);

            vkCmdBindPipeline(
                command_buffer,
                command.bind_point,
                command.pipeline
            );

            return commands_data_offset;
        }

        Commands_data_offset add_clear_color_image_command(
            VkCommandBuffer const command_buffer,
            VkImage const image,
            VkImageSubresourceRange const& image_subresource_range,
            std::span<std::byte const> const bytes
        ) noexcept
        {
            Commands_data_offset commands_data_offset = 0;

            Clear_color_image::Type const clear_subtype = read<Clear_color_image::Type>(bytes.data() + commands_data_offset);
            commands_data_offset += sizeof(Clear_color_image::Type);

            assert(clear_subtype == Clear_color_image::Type::Dependent);

            Clear_color_image::Dependent const command = read<Clear_color_image::Dependent>(bytes.data() + commands_data_offset);
            commands_data_offset += sizeof(Clear_color_image::Dependent);

            vkCmdClearColorImage(
                command_buffer,
                image,
                VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                &command.clear_color_value,
                1,
                &image_subresource_range
            );

            return commands_data_offset;
        }

        Commands_data_offset add_draw_command(
            VkCommandBuffer const command_buffer,
            std::span<std::byte const> const bytes
        ) noexcept
        {
            Commands_data_offset commands_data_offset = 0;

            Draw const command = read<Draw>(bytes.data() + commands_data_offset);
            commands_data_offset += sizeof(command);

            vkCmdDraw(
                command_buffer,
                command.vertex_count,
                command.instance_count,
                command.first_vertex,
                command.first_instance
            );

            return commands_data_offset;
        }

        Commands_data_offset add_end_render_pass_command(
            VkCommandBuffer const command_buffer
        ) noexcept
        {
            vkCmdEndRenderPass(
                command_buffer
            );

            return 0;
        }

        std::pair<Commands_data_offset, std::pmr::vector<VkImageMemoryBarrier>> create_image_memory_barriers(
            std::span<std::byte const> const bytes,
            std::uint8_t const barrier_count,
            VkImage const image,
            VkImageSubresourceRange const& image_subresource_range,
            std::pmr::polymorphic_allocator<> const& output_allocator
        ) noexcept
        {
            Commands_data_offset commands_data_offset = 0;

            std::pmr::vector<VkImageMemoryBarrier> barriers{output_allocator};
            barriers.reserve(barrier_count);

            for (std::uint8_t barrier_index = 0; barrier_index < barrier_count; ++barrier_index)
            {
                Image_memory_barrier::Type const type = read<Image_memory_barrier::Type>(bytes.data() + commands_data_offset);
                commands_data_offset += sizeof(Image_memory_barrier::Type);

                assert(type == Image_memory_barrier::Type::Dependent);

                Image_memory_barrier::Dependent const command = read<Image_memory_barrier::Dependent>(bytes.data() + commands_data_offset);
                commands_data_offset += sizeof(Image_memory_barrier::Dependent);

                barriers.push_back({
                    .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
                    .pNext = nullptr,
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

            return {commands_data_offset, barriers};
        }

        Commands_data_offset add_pipeline_barrier_command(
            VkCommandBuffer const command_buffer,
            VkImage const image,
            VkImageSubresourceRange const& image_subresource_range,
            std::span<std::byte const> const bytes,
            std::pmr::polymorphic_allocator<> const& temporaries_allocator
        ) noexcept
        {
            Commands_data_offset commands_data_offset = 0;

            Pipeline_barrier const command = read<Pipeline_barrier>(bytes.data() + commands_data_offset);
            commands_data_offset += sizeof(Pipeline_barrier);

            assert(command.memory_barrier_count == 0);
            assert(command.buffer_barrier_count == 0);

            std::pair<Commands_data_offset, std::pmr::vector<VkImageMemoryBarrier>> const image_barriers = 
                create_image_memory_barriers(
                    {bytes.data() + commands_data_offset, bytes.size() - commands_data_offset},
                    command.image_barrier_count,
                    image,
                    image_subresource_range,
                    temporaries_allocator
                );
            commands_data_offset += image_barriers.first;

            vkCmdPipelineBarrier(
                command_buffer,
                command.source_stage_mask,
                command.destination_stage_mask,
                command.dependency_flags,
                0,
                nullptr,
                0,
                nullptr,
                image_barriers.second.size(),
                image_barriers.second.data()
            );

            return commands_data_offset;
        }

        Commands_data_offset add_set_screen_viewport_and_scissors_commands(
            VkCommandBuffer const command_buffer,
            VkRect2D const render_area
        ) noexcept
        {
            {
                std::array<VkViewport, 1> const viewports
                {
                    VkViewport
                    {
                        .x = static_cast<float>(render_area.offset.x),
                        .y = static_cast<float>(render_area.offset.y),
                        .width = static_cast<float>(render_area.extent.width),
                        .height = static_cast<float>(render_area.extent.height),
                        .minDepth = 0.0f,
                        .maxDepth = 1.0f,
                    }
                };
                
                vkCmdSetViewport(command_buffer, 0, static_cast<std::uint32_t>(viewports.size()), viewports.data());
            }

            {
                std::array<VkRect2D, 1> const scissors
                {
                    VkRect2D
                    {
                        .offset = render_area.offset,
                        .extent = render_area.extent,
                    }
                };
                
                vkCmdSetScissor(command_buffer, 0, static_cast<std::uint32_t>(scissors.size()), scissors.data());
            }

            return 0;
        }
    }

    void draw(
        VkCommandBuffer const command_buffer,
        VkImage const output_image,
        VkImageSubresourceRange const& output_image_subresource_range,
        std::optional<VkFramebuffer> const output_framebuffer,
        VkRect2D const output_render_area,
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

            std::span<std::byte const> const next_command_bytes = {bytes.data() + offset_in_bytes, bytes.size() - offset_in_bytes};

            switch (command_type)
            {
            case Command_type::Begin_render_pass:
                assert(output_framebuffer.has_value());
                offset_in_bytes += add_begin_render_pass_command(command_buffer, *output_framebuffer, output_render_area, next_command_bytes);
                break;

            case Command_type::Bind_pipeline:
                offset_in_bytes += add_bind_pipeline_command(command_buffer, next_command_bytes);
                break;

            case Command_type::Draw:
                offset_in_bytes += add_draw_command(command_buffer, next_command_bytes);
                break;

            case Command_type::Clear_color_image:
                offset_in_bytes += add_clear_color_image_command(command_buffer, output_image, output_image_subresource_range, next_command_bytes);
                break;

            case Command_type::End_render_pass:
                offset_in_bytes += add_end_render_pass_command(command_buffer);
                break;

            case Command_type::Pipeline_barrier:
                offset_in_bytes += add_pipeline_barrier_command(command_buffer, output_image, output_image_subresource_range, next_command_bytes, temporaries_allocator);
                break;

            case Command_type::Set_screen_viewport_and_scissors:
                offset_in_bytes += add_set_screen_viewport_and_scissors_commands(command_buffer, output_render_area);
                break;

            default:
                assert(false && "Unrecognized command!");
                break;
            }
        }
    }
}
