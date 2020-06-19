module maia.renderer.vulkan.serializer;

import <nlohmann/json.hpp>;
import <vulkan/vulkan.h>;

import <cstddef>;
import <memory_resource>;
import <span>;
import <string>;
import <vector>;

namespace Maia::Renderer::Vulkan
{
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
            }
        }
    }
}
