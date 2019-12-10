import maia.renderer.vulkan.allocation_callbacks;
import maia.renderer.vulkan.command_buffer;
import maia.renderer.vulkan.command_pool;
import maia.renderer.vulkan.device;
import maia.renderer.vulkan.device_memory;
import maia.renderer.vulkan.fence;
import maia.renderer.vulkan.image;
import maia.renderer.vulkan.instance;
import maia.renderer.vulkan.physical_device;
import maia.renderer.vulkan.queue;


import <catch2/catch.hpp>;
import <vulkan/vulkan.h>;

import <algorithm>;
import <array>;
import <cassert>;
import <cstdint>;
import <iostream>;
import <memory_resource>;
import <span>;
import <unordered_map>;
import <vector>;

namespace Maia::Renderer::Vulkan::Unit_test
{
	SCENARIO("Initialize")
	{
		std::pmr::vector<VkLayerProperties> const layer_properties = enumerate_instance_layer_properties();

		std::cout << "Supported layers:\n\n";
		std::for_each(std::begin(layer_properties), std::end(layer_properties), 
			[](VkLayerProperties const properties) -> void { std::cout << properties << '\n'; });
		std::cout << '\n';

		std::array<char const*, 1> const enabled_layers { "VK_LAYER_KHRONOS_validation" };
		Instance const instance = create_instance(enabled_layers, {});

		std::pmr::vector<Physical_device> const physical_devices = enumerate_physical_devices(instance);

		std::cout << "Physical devices:\n\n";
		std::for_each(std::begin(physical_devices), std::end(physical_devices), 
			[](Physical_device const physical_device) -> void { std::cout << physical_device << '\n'; });
		std::cout << '\n';

		{
			Physical_device_features const physical_device_features = get_physical_device_properties(physical_devices[0]);

		}

		{
			Physical_device const physical_device = physical_devices[0];

			std::pmr::vector<Queue_family_properties> const queue_family_properties = get_physical_device_queue_family_properties(physical_device);

			std::array<float, 1> const queue_priorities{ 1.0f };

			std::pmr::vector<Device_queue_create_info> const queue_create_infos = [&queue_family_properties, &queue_priorities]() -> std::pmr::vector<Device_queue_create_info>
			{
				std::pmr::vector<Device_queue_create_info> queue_create_infos;
				queue_create_infos.reserve(queue_family_properties.size());

				assert(queue_family_properties.size() <= std::numeric_limits<std::uint32_t>::max());
				for (std::uint32_t queue_family_index = 0; queue_family_index < queue_family_properties.size(); ++queue_family_index)
				{
					Queue_family_properties const& properties = queue_family_properties[queue_family_index];

					if (has_graphics_capabilities(properties) || has_compute_capabilities(properties) || has_transfer_capabilities(properties))
					{
						queue_create_infos.push_back(
							create_device_queue_create_info(queue_family_index, 1, queue_priorities));
					}
				}

				return queue_create_infos;
			}();

			Device const device = create_device(physical_device, queue_create_infos, {});
			
			Physical_device_memory_properties const physical_device_memory_properties =
				get_phisical_device_memory_properties(physical_device);

			std::cout << "Physical device memory properties:\n\n";
			std::cout << physical_device_memory_properties;
			std::cout << '\n';


			Image const color_image = create_image(
				device,
				{},
				VK_IMAGE_TYPE_2D,
				VK_FORMAT_R8G8B8A8_UNORM,
				VkExtent3D{800, 600, 1},
				Mip_level_count{1},
				Array_layer_count{1},
				VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
				VK_IMAGE_LAYOUT_UNDEFINED
			);

			Memory_requirements const color_image_memory_requirements = get_memory_requirements(device, color_image);
			Memory_type_bits const color_image_memory_type_bits = get_memory_type_bits(color_image_memory_requirements);
			std::optional<Memory_type_index> const memory_type_index = find_memory_type(
				physical_device_memory_properties, 
				color_image_memory_type_bits,
				VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
			);

			REQUIRE(memory_type_index.has_value());

			Device_memory const device_memory =
				allocate_memory(device, color_image_memory_requirements.value.size, *memory_type_index, {});

			bind_memory(device, color_image, device_memory, 0);

			{
				std::optional<Queue_family_index> const queue_family_index = find_queue_family_with_capabilities(
					queue_family_properties,
					[](Queue_family_properties const& properties) -> bool { return has_graphics_capabilities(properties); }
				);

				REQUIRE(queue_family_index.has_value());
				
				Command_pool const command_pool = create_command_pool(
					device, 
					VK_COMMAND_POOL_CREATE_TRANSIENT_BIT,
					*queue_family_index,
					{}
				);

				std::pmr::vector<Command_buffer> const command_buffers = 
					allocate_command_buffers(
						device,
						command_pool,
						VK_COMMAND_BUFFER_LEVEL_PRIMARY,
						1,
						{}
					);

				REQUIRE(command_buffers.size() == 1);
				Command_buffer const command_buffer = command_buffers.front();

				begin_command_buffer(command_buffer, VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT, {});
				end_command_buffer(command_buffer);

				Queue const queue = get_device_queue(device, *queue_family_index, 0);
				queue_submit(queue, {}, {}, {&command_buffer, 1}, {}, {});
			}
		}
	}
}
