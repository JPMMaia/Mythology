import maia.renderer.vulkan.command_buffer;
import maia.renderer.vulkan.command_pool;
import maia.renderer.vulkan.device;
import maia.renderer.vulkan.device_memory;
import maia.renderer.vulkan.fence;
import maia.renderer.vulkan.image;
import maia.renderer.vulkan.instance;
import maia.renderer.vulkan.physical_device;
import maia.renderer.vulkan.queue;
import maia.renderer.vulkan.render_pass;

import <catch2/catch.hpp>;
import <vulkan/vulkan.h>;

import <algorithm>;
import <array>;
import <cassert>;
import <cstdint>;
import <cstring>;
import <fstream>;
import <iostream>;
import <memory_resource>;
import <span>;
import <unordered_map>;
import <vector>;

namespace Maia::Renderer::Vulkan::Unit_test
{
	namespace
	{
		Render_pass create_render_pass(VkDevice const device, VkFormat const color_image_format) noexcept
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
	}

	SCENARIO("Initialize")
	{
		std::pmr::vector<VkLayerProperties> const layer_properties = enumerate_instance_layer_properties();

		std::cout << "Supported layers:\n\n";
		std::for_each(std::begin(layer_properties), std::end(layer_properties), 
			[](VkLayerProperties const properties) -> void { std::cout << properties << '\n'; });
		std::cout << '\n';

		std::array<char const*, 1> const enabled_layers { "VK_LAYER_KHRONOS_validation" };
		Instance const instance = create_instance(enabled_layers, {});

		std::pmr::vector<VkPhysicalDevice> const physical_devices = enumerate_physical_devices(instance);

		std::cout << "Physical devices:\n\n";
		std::for_each(std::begin(physical_devices), std::end(physical_devices), 
			[](VkPhysicalDevice const physical_device) -> void { std::cout << physical_device << '\n'; });
		std::cout << '\n';

		{
			Physical_device_features const physical_device_features = get_physical_device_properties(physical_devices[0]);

		}

		{
			VkPhysicalDevice const physical_device = physical_devices[0];

			std::pmr::vector<VkQueueFamilyProperties> const queue_family_properties = get_physical_device_queue_family_properties(physical_device);

			std::array<float, 1> const queue_priorities{ 1.0f };

			std::pmr::vector<VkDeviceQueueCreateInfo> const queue_create_infos = [&queue_family_properties, &queue_priorities]() -> std::pmr::vector<VkDeviceQueueCreateInfo>
			{
				std::pmr::vector<VkDeviceQueueCreateInfo> queue_create_infos;
				queue_create_infos.reserve(queue_family_properties.size());

				assert(queue_family_properties.size() <= std::numeric_limits<std::uint32_t>::max());
				for (std::uint32_t queue_family_index = 0; queue_family_index < queue_family_properties.size(); ++queue_family_index)
				{
					VkQueueFamilyProperties const& properties = queue_family_properties[queue_family_index];

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


			VkFormat const color_image_format = VK_FORMAT_R8G8B8A8_UINT; 
			VkExtent3D const color_image_extent {16, 16, 1};
			Image const color_image = create_image(
				device,
				{},
				VK_IMAGE_TYPE_2D,
				color_image_format,
				color_image_extent,
				Mip_level_count{1},
				Array_layer_count{1},
				VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
				VK_IMAGE_LAYOUT_UNDEFINED,
				VK_SAMPLE_COUNT_1_BIT,
				{},
				VK_IMAGE_TILING_LINEAR
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

			Image_view const color_image_view = create_image_view(
				device,
				{},
				color_image,
				VK_IMAGE_VIEW_TYPE_2D,
				color_image_format,
				{},
				{VkImageAspectFlagBits::VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1},
				{}
			);

			Render_pass const render_pass = create_render_pass(
				device,
				color_image_format
			);

			Framebuffer const framebuffer = create_framebuffer(
				device,
				{},
				render_pass,
				{&color_image_view.value, 1},
				Framebuffer_dimensions{color_image_extent.width, color_image_extent.height, 1},
				{}
			);

			{
				std::optional<Queue_family_index> const queue_family_index = find_queue_family_with_capabilities(
					queue_family_properties,
					[](VkQueueFamilyProperties const& properties) -> bool { return has_graphics_capabilities(properties); }
				);

				REQUIRE(queue_family_index.has_value());
				
				Command_pool const command_pool = create_command_pool(
					device, 
					VK_COMMAND_POOL_CREATE_TRANSIENT_BIT,
					*queue_family_index,
					{}
				);

				std::pmr::vector<VkCommandBuffer> const command_buffers = 
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
				{
					VkClearValue const clear_value
					{
						.color = {
							.uint32 =  {0, 0, 128, 255}
						}
					};

					{
						VkImageSubresourceRange const color_image_subresource_range
						{
							.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT, 
							.baseMipLevel = 0,
							.levelCount = 1, 
							.baseArrayLayer = 0,
							.layerCount = 1
						};

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
								.image = color_image.value,
								.subresourceRange = color_image_subresource_range
							};

							vkCmdPipelineBarrier(
								command_buffer,
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

						vkCmdClearColorImage(
							command_buffer,
							color_image.value,
							VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 
							&clear_value.color, 
							1,
							&color_image_subresource_range
						);
						
							
					}

					/*vkCmdClearColorImage(
						command_buffer,
						color_image.value,
						VK_IMAGE_, const VkClearColorValue *pColor, uint32_t rangeCount, const VkImageSubresourceRange *pRanges)
					*/

					/*begin_render_pass(
						command_buffer,
						render_pass,
						framebuffer,
						{static_cast<int32_t>(color_image_extent.width), static_cast<int32_t>(color_image_extent.height)},
						{&clear_value, 1},
						VK_SUBPASS_CONTENTS_INLINE
					);

					end_render_pass(
						command_buffer
					);*/
				}
				end_command_buffer(command_buffer);

				VkFence const fence = create_fence(device, {}, {});

				Queue const queue = get_device_queue(device, *queue_family_index, 0);
				queue_submit(queue, {}, {}, {&command_buffer, 1}, {}, fence);

				REQUIRE(
					wait_for_all_fences(device, {&fence, 1}, Timeout_nanoseconds{100000}) == VK_SUCCESS
				);

				{
					VkSubresourceLayout const color_image_layout = get_subresource_layout(
						device,
						color_image,
						{.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT, .mipLevel = 0, .arrayLayer = 0}
					);

					{
						void* const data = map_memory(
							device,
							device_memory,
							color_image_layout.offset,
							color_image_layout.size
						);

						{
							std::ofstream output_stream{"test_image.ppm"};

							output_stream << "P3\n";
							output_stream << color_image_extent.width << ' ' << color_image_extent.height << '\n';
							output_stream << "255\n";

							for (std::uint32_t row_index = 0; row_index < color_image_extent.height; ++row_index)
							{
								for (std::uint32_t column_index = 0; column_index < color_image_extent.width; ++column_index)
								{
									std::uint64_t const texel_data_offset =
										row_index * color_image_layout.rowPitch + 4 * column_index;
									void const* const texel_data = static_cast<std::byte*>(data) + texel_data_offset;

									std::array<char8_t, 4> color = {};
									std::memcpy(color.data(), texel_data, sizeof(decltype(color)::value_type) * color.size());

									output_stream << color[0] << ' ';
									output_stream << color[1] << ' ';
									output_stream << color[2] << "  ";
								}
								output_stream << '\n';
							}
						}

						vkUnmapMemory(
							device,
							device_memory.value
						);
					}
				}
				
			}
		}
	}
}
