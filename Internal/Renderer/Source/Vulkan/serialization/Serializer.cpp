module maia.renderer.vulkan.serializer;

import maia.renderer.vulkan.check;

import <nlohmann/json.hpp>;
import <vulkan/vulkan.h>;

import <cstddef>;
import <filesystem>;
import <fstream>;
import <memory_resource>;
import <span>;
import <string>;
import <vector>;

namespace Maia::Renderer::Vulkan
{
    namespace
    {
        std::pmr::vector<VkAttachmentDescription> create_attachments(
            nlohmann::json const& attachments_json,
            std::pmr::polymorphic_allocator<VkAttachmentDescription> const& allocator
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

            std::pmr::vector<VkAttachmentDescription> attachments{allocator};
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
            std::pmr::polymorphic_allocator<VkAttachmentReference> const& allocator
        ) noexcept
        {
            std::pmr::vector<VkAttachmentReference> attachment_references{allocator};
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
            std::pmr::polymorphic_allocator<VkAttachmentReference> const& allocator
        ) noexcept
        {
            std::pmr::vector<VkAttachmentReference> attachment_references{allocator};
            attachment_references.reserve(get_attachment_reference_count(subpasses_json));

            for (nlohmann::json const& subpass_json : subpasses_json)
            {
                assert(subpass_json.at("resolve_attachments").empty() || (subpass_json.at("resolve_attachments").size() == subpass_json.at("color_attachments").size()));

                std::pmr::vector<VkAttachmentReference> const input_attachments = 
                    create_attachment_references(subpass_json.at("input_attachments"), allocator);
                attachment_references.insert(attachment_references.end(), input_attachments.begin(), input_attachments.end());

                std::pmr::vector<VkAttachmentReference> const color_attachments = 
                    create_attachment_references(subpass_json.at("color_attachments"), allocator);
                attachment_references.insert(attachment_references.end(), color_attachments.begin(), color_attachments.end());

                std::pmr::vector<VkAttachmentReference> const resolve_attachments =
                    create_attachment_references(subpass_json.at("resolve_attachments"), allocator);
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
            std::pmr::polymorphic_allocator<std::uint32_t> const& allocator
        ) noexcept
        {
            std::pmr::vector<std::uint32_t> preserve_attachments{allocator};

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
            std::pmr::polymorphic_allocator<VkSubpassDescription> const& allocator
        ) noexcept
        {
            std::pmr::vector<VkSubpassDescription> subpasses{allocator};
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
            std::pmr::polymorphic_allocator<VkSubpassDependency> const& allocator
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

            std::pmr::vector<VkSubpassDependency> dependencies{allocator};
            dependencies.resize(dependencies_json.size());

            std::transform(dependencies_json.begin(), dependencies_json.end(), dependencies.begin(), create_dependency);

            return dependencies;
        }
    }

    Render_pass_create_info_resources create_render_pass_create_info_resources(
        nlohmann::json const& render_pass_json,
        std::pmr::polymorphic_allocator<VkAttachmentDescription> const& attachments_allocator,
        std::pmr::polymorphic_allocator<VkAttachmentReference> const& attachment_reference_allocator,
        std::pmr::polymorphic_allocator<std::uint32_t> const& preserve_attachment_allocator,
        std::pmr::polymorphic_allocator<VkSubpassDescription> const& subpasses_allocator,
        std::pmr::polymorphic_allocator<VkSubpassDependency> const& dependencies_allocator
    ) noexcept
    {
        std::pmr::vector<VkAttachmentDescription> attachments = create_attachments(render_pass_json.at("attachments"), attachments_allocator);
        std::pmr::vector<VkAttachmentReference> attachment_references = create_subpasses_attachment_references(render_pass_json.at("subpasses"), attachment_reference_allocator);
        std::pmr::vector<std::uint32_t> preserve_attachments = create_subpasses_preserve_attachments(render_pass_json.at("subpasses"), preserve_attachment_allocator);
        std::pmr::vector<VkSubpassDescription> subpasses = create_subpasses(render_pass_json.at("subpasses"), attachment_references, preserve_attachments, subpasses_allocator);
        std::pmr::vector<VkSubpassDependency> dependencies = create_dependencies(render_pass_json.at("dependencies"), dependencies_allocator);
        
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
        std::pmr::polymorphic_allocator<VkAttachmentDescription> const& attachments_allocator,
        std::pmr::polymorphic_allocator<VkAttachmentReference> const& attachment_reference_allocator,
        std::pmr::polymorphic_allocator<std::uint32_t> const& preserve_attachment_allocator,
        std::pmr::polymorphic_allocator<VkSubpassDescription> const& subpasses_allocator,
        std::pmr::polymorphic_allocator<VkSubpassDependency> const& dependencies_allocator,
        std::pmr::polymorphic_allocator<VkRenderPass> const& allocator
    ) noexcept
    {
        std::pmr::vector<VkRenderPass> render_passes{allocator};
        render_passes.reserve(render_passes_json.size());

        for (nlohmann::json const& render_pass_json : render_passes_json)       
        {
            Render_pass_create_info_resources const create_info_resources = create_render_pass_create_info_resources(
                render_pass_json,
                attachments_allocator,
                attachment_reference_allocator,
                preserve_attachment_allocator,
                subpasses_allocator,
                dependencies_allocator
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
            std::pmr::polymorphic_allocator<> const& allocator = {}
        ) noexcept
        {
            std::ifstream input_stream{file_path, std::ios::in | std::ios::binary};
            assert(input_stream.good());

            input_stream.seekg(0, std::ios::end);
            auto const size_in_bytes = input_stream.tellg();

            std::pmr::vector<std::byte> buffer{allocator};
            buffer.resize(size_in_bytes);

            input_stream.seekg(0, std::ios::beg);
            input_stream.read(reinterpret_cast<char*>(buffer.data()), buffer.size());

            return buffer;
        }

        template<typename Value_type>
        std::pmr::vector<Value_type> convert_bytes(
            std::span<std::byte const> const bytes,
            std::pmr::polymorphic_allocator<> const& allocator = {}
        ) noexcept
        {
            assert(bytes.size_bytes() % sizeof(Value_type) == 0);

            std::pmr::vector<Value_type> values{allocator};
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
        std::pmr::polymorphic_allocator<> const& allocator
    ) noexcept
    {
        std::pmr::vector<VkShaderModule> shader_modules{allocator};
        shader_modules.reserve(shader_modules_json.size());

        for (nlohmann::json const& shader_module_json : shader_modules_json)       
        {
            std::string const& shader_file = shader_module_json.at("file").get<std::string>();

            std::pmr::vector<std::uint32_t> const shader_code = convert_bytes<std::uint32_t>(read_bytes(shaders_path / shader_file, allocator), allocator);

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
        std::pmr::polymorphic_allocator<> const& allocator
    ) noexcept
    {
        std::pmr::vector<VkSampler> samplers{allocator};
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
            std::pmr::polymorphic_allocator<> const& allocator
        ) noexcept
        {
            std::pmr::vector<VkSampler> immutable_samplers_per_descriptor_set_binding{allocator};

            for (nlohmann::json const& descriptor_set_layout_json : descriptor_set_layouts_json)       
            {
                for (nlohmann::json const& immutable_sampler_json : descriptor_set_layout_json.at("immutable_samplers"))
                {
                    std::size_t const sampler_index = immutable_sampler_json.get<std::size_t>();

                    immutable_samplers_per_descriptor_set_binding.push_back(
                        samplers[sampler_index]
                    );
                }
            }

            return immutable_samplers_per_descriptor_set_binding;
        }

        std::pmr::vector<VkDescriptorSetLayoutBinding> create_descriptor_set_layouts_bindings(
            nlohmann::json const& descriptor_set_layouts_json,
            std::span<VkSampler const> const immutable_samplers_per_descriptor_set_binding,
            std::pmr::polymorphic_allocator<> const& allocator
        ) noexcept
        {
            std::pmr::vector<VkDescriptorSetLayoutBinding> bindings{allocator};

            std::uint32_t start_sampler_index = 0;

            for (nlohmann::json const& descriptor_set_layout_json : descriptor_set_layouts_json)       
            {
                nlohmann::json const& immutable_samplers_json = descriptor_set_layout_json.at("immutable_samplers");

                VkDescriptorSetLayoutBinding const binding
                {
                    .binding = descriptor_set_layout_json.at("binding").get<std::uint32_t>(),
                    .descriptorType = descriptor_set_layout_json.at("descriptor_type").get<VkDescriptorType>(),
                    .descriptorCount = descriptor_set_layout_json.at("descriptor_count").get<std::uint32_t>(),
                    .stageFlags = descriptor_set_layout_json.at("stage_flags_property").get<VkShaderStageFlags>(),
                    .pImmutableSamplers = 
                        !immutable_samplers_json.empty() ?
                        immutable_samplers_per_descriptor_set_binding.data() + start_sampler_index :
                        nullptr
                };

                bindings.push_back(binding);

                start_sampler_index += immutable_samplers_json.size();
            }

            return bindings;
        }
    }

    std::pmr::vector<VkDescriptorSetLayout> create_descriptor_set_layouts(
        VkDevice const device,
        VkAllocationCallbacks const* const allocation_callbacks,
        std::span<VkSampler const> const samplers,
        nlohmann::json const& descriptor_set_layouts_json,
        std::pmr::polymorphic_allocator<> const& allocator
    ) noexcept
    {
        std::pmr::vector<VkSampler> const immutable_samplers_per_descriptor_set_binding =
            arrange_immutable_samplers(descriptor_set_layouts_json, samplers, allocator);

        std::pmr::vector<VkDescriptorSetLayoutBinding> const bindings = 
            create_descriptor_set_layouts_bindings(descriptor_set_layouts_json, immutable_samplers_per_descriptor_set_binding, allocator);

        std::pmr::vector<VkDescriptorSetLayout> descriptor_set_layouts{allocator};
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
            std::pmr::polymorphic_allocator<> const& allocator
        ) noexcept
        {
            std::pmr::vector<VkDescriptorSetLayout> descriptor_set_layouts_per_pipeline_layout{allocator};

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
            std::pmr::polymorphic_allocator<> const& allocator
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

            std::pmr::vector<VkPushConstantRange> push_constant_ranges{allocator};

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
        std::pmr::polymorphic_allocator<> const& allocator
    ) noexcept
    {
        std::pmr::vector<VkDescriptorSetLayout> const descriptor_set_layouts_per_pipeline_layout =
            arrange_descriptor_set_layouts(pipeline_layouts_json, descriptor_set_layouts, allocator);

        std::pmr::vector<VkPushConstantRange> const push_constant_ranges =
            create_push_constant_ranges(pipeline_layouts_json, allocator);

        std::pmr::vector<VkPipelineLayout> pipeline_layouts{allocator};
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
        enum class Command_type : std::uint8_t
        {
            Clear_color_image,
            Pipeline_barrier
        };

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
            std::pmr::polymorphic_allocator<std::byte> const& allocator
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

            std::pmr::vector<std::byte> data{allocator};
            data.resize(sizeof(Image_memory_barrier::Type) + sizeof(Image_memory_barrier::Dependent));
            std::memcpy(data.data(), &barrier_type, sizeof(barrier_type));
            std::memcpy(data.data() + sizeof(Image_memory_barrier::Type), &dependent, sizeof(dependent));
            return data;
        }

        std::pmr::vector<std::byte> create_pipeline_barrier(
            nlohmann::json const& command_json,
            std::pmr::polymorphic_allocator<std::byte> const& allocator
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

            std::pmr::vector<std::byte> data{allocator};
            data.resize(sizeof(Command_type) + sizeof(Pipeline_barrier));
            std::memcpy(data.data(), &command_type, sizeof(command_type));
            std::memcpy(data.data() + sizeof(Command_type), &pipeline_barrier, sizeof(pipeline_barrier));

            for (nlohmann::json const& image_barrier_json : command_json.at("image_barriers"))
            {
                std::pmr::vector<std::byte> const image_memory_barrier_data = 
                    create_image_memory_barrier(image_barrier_json, allocator);

                data.insert(data.end(), image_memory_barrier_data.begin(), image_memory_barrier_data.end());
            }

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
            std::pmr::polymorphic_allocator<std::byte> const& allocator
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

            std::pmr::vector<std::byte> data{allocator};
            data.resize(sizeof(Command_type) + sizeof(Clear_color_image::Type) + sizeof(Clear_color_image::Dependent));
            std::memcpy(data.data(), &command_type, sizeof(command_type));
            std::memcpy(data.data() + sizeof(command_type), &clear_subtype, sizeof(clear_subtype));
            std::memcpy(data.data() + sizeof(command_type) + sizeof(clear_subtype), &dependent, sizeof(dependent));
            return data;
        }

        std::pmr::vector<std::byte> create_command_data(
            nlohmann::json const& command_json,
            std::pmr::polymorphic_allocator<std::byte> const& allocator
        ) noexcept
        {
            assert(command_json.contains("type"));
            std::string const& type = command_json.at("type").get<std::string>();

            if (type == "Clear_color_image")
            {
                return create_color_image_data(command_json, allocator);
            }
            else if (type == "Pipeline_barrier")
            {
                return create_pipeline_barrier(command_json, allocator);
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
        std::pmr::polymorphic_allocator<std::byte> const& commands_allocator
    ) noexcept
    {
        std::pmr::vector<std::byte> commands_data{commands_allocator};

        for (nlohmann::json const& draw_list_json : commands_json)
        {
            for (nlohmann::json const& command_json : draw_list_json)
            {
                std::pmr::vector<std::byte> const command_data = create_command_data(command_json, commands_allocator);

                commands_data.insert(commands_data.end(), command_data.begin(), command_data.end());
            }
        }

        return {commands_data};
    }

    namespace
    {
        using Commands_data_offset = size_t;

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

        std::pair<Commands_data_offset, std::pmr::vector<VkImageMemoryBarrier>> create_image_memory_barriers(
            std::span<std::byte const> const bytes,
            std::uint8_t const barrier_count,
            VkImage const image,
            VkImageSubresourceRange const& image_subresource_range,
            std::pmr::polymorphic_allocator<VkImageMemoryBarrier> const& allocator
        ) noexcept
        {
            Commands_data_offset commands_data_offset = 0;

            std::pmr::vector<VkImageMemoryBarrier> barriers{allocator};
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
            std::pmr::polymorphic_allocator<VkImageMemoryBarrier> const& image_memory_barrier_allocator
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
                    image_memory_barrier_allocator
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
    }

    void draw(
        VkCommandBuffer const command_buffer,
        VkImage const output_image,
        VkImageSubresourceRange const& output_image_subresource_range,
        Commands_data const& commands_data
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
            case Command_type::Clear_color_image:
                offset_in_bytes += add_clear_color_image_command(command_buffer, output_image, output_image_subresource_range, next_command_bytes);
                break;

            case Command_type::Pipeline_barrier:
                offset_in_bytes += add_pipeline_barrier_command(command_buffer, output_image, output_image_subresource_range, next_command_bytes, {});
                break;

            default:
                assert(false && "Unrecognized command!");
                break;
            }
        }
    }
}
