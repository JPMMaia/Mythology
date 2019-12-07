import maia.renderer.vulkan.device;
import maia.renderer.vulkan.device_memory;
import maia.renderer.vulkan.physical_device;
import maia.renderer.vulkan.instance;

import <catch2/catch.hpp>;
import <vulkan/vulkan.h>;

import <algorithm>;
import <array>;
import <cassert>;
import <cstdint>;
import <iostream>;
import <memory_resource>;
import <span>;
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
		}
	}
}
