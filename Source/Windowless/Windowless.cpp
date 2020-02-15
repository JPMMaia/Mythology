module mythology.windowless;

import mythology.core.vulkan;
import maia.renderer.vulkan;

import <vulkan/vulkan.h>;

import <cassert>;
import <filesystem>;
import <fstream>;
import <memory_resource>;
import <optional>;
import <span>;
import <vector>;

namespace Mythology::Windowless
{
    void render_frame(
        std::filesystem::path const& output_filename
    ) noexcept
    {
        using namespace Mythology::Core::Vulkan;
        using namespace Maia::Renderer::Vulkan;

        Instance const instance = create_instance();
        Physical_device const physical_device = select_physical_device(instance);
        Device const device = create_device(physical_device);
        Queue_family_index const graphics_queue_family_index = find_graphics_queue_family_index(physical_device);
        Command_pool const command_pool = create_command_pool(device, VK_COMMAND_POOL_CREATE_TRANSIENT_BIT, graphics_queue_family_index, {});
        Queue const queue = get_device_queue(device, graphics_queue_family_index, 0);
        Fence const fence = create_fence(device, {}, {});

        VkExtent3D constexpr color_image_extent{16, 16, 1};
        Device_memory_and_color_image const device_memory_and_color_image = 
            create_device_memory_and_color_image(physical_device, device, VK_FORMAT_R8G8B8A8_UINT, color_image_extent);

        {
            std::pmr::vector<Command_buffer> const command_buffers = 
                allocate_command_buffers(
                    device,
                    command_pool,
                    VK_COMMAND_BUFFER_LEVEL_PRIMARY,
                    1,
                    {}
                );
            assert(command_buffers.size() == 1);
            Command_buffer const command_buffer = command_buffers.front();

            begin_command_buffer(command_buffer, VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT, {});
            {
                render(command_buffer, device_memory_and_color_image.color_image);
            }
            end_command_buffer(command_buffer);

            queue_submit(queue, {}, {}, {&command_buffer, 1}, {}, fence);
        }
        check_result(
            wait_for_all_fences(device, {&fence, 1}, Timeout_nanoseconds{100000}));

        {
            VkSubresourceLayout const color_image_layout = get_subresource_layout(
                device,
                device_memory_and_color_image.color_image,
                {.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT, .mipLevel = 0, .arrayLayer = 0}
            );

            std::pmr::vector<std::byte> const color_image_data = 
                read_memory(device, device_memory_and_color_image.device_memory, color_image_layout);

            std::ofstream output_file{output_filename};
            write_p3(output_file, color_image_data, color_image_layout, color_image_extent);
        }
    }
}