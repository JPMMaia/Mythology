module mythology.sdl.application;

import maia.input;
import maia.renderer.vulkan;
import maia.renderer.vulkan.serializer;
import maia.sdl.vulkan;
import maia.utilities.gltf;
import mythology.core.utilities;
import mythology.core.vulkan;
import mythology.imgui;

import <imgui.h>;

import <nlohmann/json.hpp>;

import <SDL2/SDL.h>;

import <vulkan/vulkan.h>;

import <algorithm>;
import <cassert>;
import <cstdlib>;
import <cstring>;
import <filesystem>;
import <functional>;
import <iostream>;
import <memory_resource>;
import <optional>;
import <span>;
import <utility>;
import <variant>;
import <vector>;

import <fstream>;
import <future>;

using namespace Maia::SDL::Vulkan;
using namespace Maia::Renderer::Vulkan;
using namespace Mythology::Core;
using namespace Mythology::Core::Vulkan;

namespace Mythology::SDL
{
    namespace
    {
        nlohmann::json read_json_from_file(std::filesystem::path const& path) noexcept
        {
            std::ifstream input_stream{path};
            assert(input_stream.good());

            nlohmann::json json{};
            input_stream >> json;

            return json;
        }

        class SDL_application
        {
        public:

            SDL_application() noexcept
            {
                SDL_SetMainReady();
            }
            SDL_application(SDL_application const&) noexcept = delete;
            SDL_application(SDL_application&&) noexcept = delete;
            ~SDL_application() noexcept
            {
                SDL_Quit();
            }

            SDL_application& operator=(SDL_application const&) noexcept = delete;
            SDL_application& operator=(SDL_application&&) noexcept = delete;
        };

        class SDL_window
        {
        public:

            SDL_window(
                char const* const title,
                int const x,
                int const y,
                int const w,
                int const h,
                Uint32 const flags
            ) noexcept :
                m_window{SDL_CreateWindow(title, x, y, w, h, flags)}
            {
            }
            SDL_window(SDL_window const&) noexcept = delete;
            SDL_window(SDL_window&& other) noexcept :
                m_window{std::exchange(other.m_window, nullptr)}
            {
            }
            ~SDL_window() noexcept
            {
                if (m_window != nullptr)
                {
                    SDL_DestroyWindow(m_window);
                }
            }

            SDL_window& operator=(SDL_window const&) noexcept = delete;
            SDL_window& operator=(SDL_window&& other) noexcept
            {
                std::swap(m_window, other.m_window);
                
                return *this;
            }

            SDL_Window* get() const
            {
                return m_window;
            }

        private:

            SDL_Window* m_window = nullptr;
        };

        void toggle_fullscreen(SDL_Window& window) noexcept
        {
            SDL_WindowFlags constexpr fullscreen_flag = SDL_WINDOW_FULLSCREEN;
            Uint32 constexpr windowed_flag = 0;

            bool const is_fullscreen = SDL_GetWindowFlags(&window) & fullscreen_flag;

            if (SDL_SetWindowFullscreen(&window, is_fullscreen ? windowed_flag : fullscreen_flag) != 0)
            {
                std::cerr << SDL_GetError() << '\n';
            }
        }

        std::uint32_t select_swapchain_image_count(
            VkSurfaceCapabilitiesKHR const& surface_capabilities
        ) noexcept
        {
            std::uint32_t const desired_image_count = surface_capabilities.minImageCount + 1;

            if (surface_capabilities.maxImageCount != 0)
            {
                return std::clamp(desired_image_count, surface_capabilities.minImageCount, surface_capabilities.maxImageCount);
            }
            else
            {
                return std::max(desired_image_count, surface_capabilities.minImageCount);
            }
        }

        VkSurfaceFormatKHR select_surface_format(
            VkPhysicalDevice const physical_device,
            Surface const surface
        ) noexcept
        {
            std::pmr::vector<VkSurfaceFormatKHR> const supported_surface_formats =
                get_surface_formats(physical_device, {surface});

            return supported_surface_formats[0];
        }

        VkImageUsageFlags select_swapchain_usage_flags(
            VkSurfaceCapabilitiesKHR const& surface_capabilities
        ) noexcept
        {
            VkImageUsageFlags const swapchain_usage_flags = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
            assert((swapchain_usage_flags & surface_capabilities.supportedUsageFlags) == swapchain_usage_flags);
            return swapchain_usage_flags;
        }

        VkCompositeAlphaFlagBitsKHR select_composite_alpha(
            VkSurfaceCapabilitiesKHR const& surface_capabilities
        ) noexcept
        {
            VkCompositeAlphaFlagBitsKHR const selected_composite_alpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
            assert((selected_composite_alpha & surface_capabilities.supportedCompositeAlpha) == selected_composite_alpha);
            return selected_composite_alpha;
        }

        VkPresentModeKHR select_present_mode(
            VkPhysicalDevice const physical_device,
            Surface const surface
        ) noexcept
        {
            return VK_PRESENT_MODE_FIFO_KHR;
        }

        VkSharingMode select_sharing_mode(std::span<Queue_family_index const> const queue_family_indices) noexcept
        {
            assert(!queue_family_indices.empty());

            if (std::equal(queue_family_indices.begin() + 1, queue_family_indices.end(), queue_family_indices.begin()))
            {
                return VK_SHARING_MODE_EXCLUSIVE;
            }
            else
            {
                return VK_SHARING_MODE_CONCURRENT;
            }
        }

        std::span<Queue_family_index const> select_queue_family_indices(
            std::span<Queue_family_index const> const queue_family_indices,
            VkSharingMode const sharing_mode) noexcept
        {
            if (sharing_mode == VK_SHARING_MODE_EXCLUSIVE)
            {
                return {&queue_family_indices[0], 1};
            }
            else
            {
                return queue_family_indices;
            }
        }

        Swapchain create_swapchain(
            VkPhysicalDevice const physical_device,
            Device const device,
            Surface const surface,
            VkSurfaceFormatKHR const surface_format,
            std::span<Queue_family_index const> const queue_family_indices,
            std::optional<Swapchain> const old_swapchain = {}
        ) noexcept
        {
            assert(!queue_family_indices.empty());

            VkSurfaceCapabilitiesKHR const surface_capabilities = get_surface_capabilities(physical_device, surface);
            std::uint32_t const image_count = select_swapchain_image_count(surface_capabilities);
            VkImageUsageFlags const swapchain_usage_flags = select_swapchain_usage_flags(surface_capabilities);
            VkCompositeAlphaFlagBitsKHR const composite_alpha = select_composite_alpha(surface_capabilities);
            VkPresentModeKHR const present_mode = select_present_mode(physical_device, surface);
            VkSharingMode const sharing_mode = select_sharing_mode(queue_family_indices);
            std::span<Queue_family_index const> const shared_queue_family_indices = select_queue_family_indices(queue_family_indices, sharing_mode);

            return create_swapchain(
                device,
                VkSwapchainCreateFlagsKHR{},
                {image_count},
                {surface},
                surface_format.format,
                surface_format.colorSpace,
                surface_capabilities.currentExtent,
                Array_layer_count{1},
                swapchain_usage_flags,
                sharing_mode,
                shared_queue_family_indices,
                surface_capabilities.currentTransform,
                composite_alpha,
                present_mode,
                true,
                old_swapchain.has_value() ? *old_swapchain : Swapchain{VK_NULL_HANDLE}
            );
        }

        std::pmr::vector<VkImageView> create_swapchain_image_views(
            Device const device,
            std::span<VkImage const> const images,
            VkFormat const format
        ) noexcept
        {
            auto const create_image_view = [&](VkImage const image) -> VkImageView
            {
                return Maia::Renderer::Vulkan::create_image_view(
                    device,
                    {},
                    {image},
                    VK_IMAGE_VIEW_TYPE_2D,
                    format,
                    {},
                    {VkImageAspectFlagBits::VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1},
                    {}
			    ).value;
            };

            std::pmr::vector<VkImageView> image_views;
            image_views.resize(images.size());

            std::transform(images.begin(), images.end(), image_views.begin(), create_image_view);

            return image_views;
        }

        struct Frame_index
        {
            std::uint8_t value = 0;
        };

        struct Device_resources
        {
            Device_resources(API_version const api_version, SDL_Window& window) noexcept
            {
                using namespace Mythology::Core::Vulkan;

                std::pmr::vector<char const*> const required_instance_extensions = get_sdl_required_instance_extensions(window);
                this->instance = create_instance(Application_description{"Mythology", 1}, Engine_description{"Mythology Engine", 1}, api_version, required_instance_extensions);

                this->physical_device = select_physical_device(this->instance);
                
                this->surface = {create_surface(window, this->instance)};
                
                this->graphics_queue_family_index = find_graphics_queue_family_index(this->physical_device);
                this->present_queue_family_index = find_present_queue_family_index(this->physical_device, this->surface, graphics_queue_family_index);

                auto const is_extension_to_enable = [](VkExtensionProperties const& properties) -> bool
                {
                    return std::strcmp(properties.extensionName, VK_KHR_SWAPCHAIN_EXTENSION_NAME) == 0;
                };

                if (this->graphics_queue_family_index != this->present_queue_family_index)
                {
                    std::array<Queue_family_index, 2> const queue_family_indices{this->graphics_queue_family_index, this->present_queue_family_index};
                    this->device = create_device(this->physical_device, queue_family_indices, is_extension_to_enable);
                }
                else
                {
                    Queue_family_index const queue_family_index = this->present_queue_family_index;
                    this->device = create_device(this->physical_device, {&queue_family_index, 1}, is_extension_to_enable);
                }
            }
            Device_resources(Device_resources const&) = delete;
            Device_resources(Device_resources&&) = delete;
            ~Device_resources() noexcept
            {
                if (this->device.value != VK_NULL_HANDLE)
                {
                    destroy_device(this->device);
                }

                if (this->surface.value != VK_NULL_HANDLE)
                {
                    destroy_surface(this->instance, this->surface);
                }

                if (this->instance != VK_NULL_HANDLE)
                {
                    destroy_instance(this->instance);
                }
            }

            Device_resources& operator=(Device_resources const&) = delete;
            Device_resources& operator=(Device_resources&&) = delete;

            VkInstance instance = {};
            VkPhysicalDevice physical_device = {};
            Surface surface = {};
            Queue_family_index graphics_queue_family_index = {};
            Queue_family_index present_queue_family_index = {};
            Device device = {};
        };

        std::pmr::vector<Framebuffer> create_swapchain_framebuffers(
            Device const device,
            Render_pass const render_pass,
            std::span<VkImageView const> const swapchain_image_views,
            Framebuffer_dimensions const framebuffer_dimensions
        ) noexcept
        {
            std::pmr::vector<Framebuffer> framebuffers;
            framebuffers.reserve(swapchain_image_views.size());
            
            for (VkImageView const image_view : swapchain_image_views)
            {
                framebuffers.push_back(create_framebuffer(device, {}, render_pass, {&image_view, 1}, framebuffer_dimensions, {}));
            }

            return framebuffers;
        }

        struct Swapchain_resources
        {
            Swapchain_resources(
                VkPhysicalDevice const physical_device,
                Device const device,
                Surface const surface,
                VkSurfaceFormatKHR const surface_format,
                std::span<Queue_family_index const> const queue_family_indices,
                Render_pass const render_pass,
                std::optional<Swapchain> const old_swapchain = {}) noexcept :
                device{device},
                swapchain{create_swapchain(physical_device, device, surface, surface_format, queue_family_indices, old_swapchain)},
                images{get_swapchain_images(device, this->swapchain)},
                image_views{create_swapchain_image_views(device, this->images, surface_format.format)},
                extent{get_surface_capabilities(physical_device, surface).currentExtent},
                framebuffers{create_swapchain_framebuffers(device, render_pass, this->image_views, {this->extent.width, this->extent.height, 1})}
            {
            }
            Swapchain_resources(Swapchain_resources const&) = delete;
            Swapchain_resources(Swapchain_resources&& other) noexcept :
                device{std::exchange(other.device, {VK_NULL_HANDLE})},
                swapchain{std::exchange(other.swapchain, {VK_NULL_HANDLE})},
                images{std::exchange(other.images, {})},
                image_views{std::exchange(other.image_views, {})},
                extent{other.extent},
                framebuffers{std::exchange(other.framebuffers, {})}
            {
            }
            ~Swapchain_resources() noexcept
            {
                for (Framebuffer const framebuffer : this->framebuffers)
                {
                    destroy_framebuffer(this->device, framebuffer, {});
                }

                for (VkImageView const image_view : this->image_views)
                {
                    vkDestroyImageView(this->device.value, image_view, nullptr);
                }

                if (this->swapchain.value != VK_NULL_HANDLE)
                {
                    destroy_swapchain(this->device, this->swapchain);
                }
            }

            Swapchain_resources& operator=(Swapchain_resources const&) = delete;
            Swapchain_resources& operator=(Swapchain_resources&& other) noexcept
            {
                this->device = other.device;
                std::swap(this->swapchain, other.swapchain);
                std::swap(this->images, other.images);
                std::swap(this->image_views, other.image_views);
                this->extent = other.extent;
                std::swap(this->framebuffers, other.framebuffers);

                return *this;
            }

            Device device = {};
            Swapchain swapchain = {};
            std::pmr::vector<VkImage> images;
            std::pmr::vector<VkImageView> image_views;
            VkExtent2D extent = {};
            std::pmr::vector<Framebuffer> framebuffers;
        };

        struct Command_pools_resources
        {
            Command_pools_resources(
                Device const device,
                VkCommandPoolCreateFlags const flags,
                Queue_family_index const queue_family_index) noexcept :
                device{device},
                command_pool{create_command_pool(device, flags, queue_family_index, {})}
            {
            }
            Command_pools_resources(Command_pools_resources const&) noexcept = delete;
            Command_pools_resources(Command_pools_resources&&) noexcept = delete;
            ~Command_pools_resources() noexcept
            {
                if (this->command_pool.value != VK_NULL_HANDLE)
                {
                    destroy_command_pool(this->device, this->command_pool, {});
                }
            }

            Command_pools_resources& operator=(Command_pools_resources const&) noexcept = delete;
            Command_pools_resources& operator=(Command_pools_resources&&) noexcept = delete;

            Device device = {};
            Command_pool command_pool = {};
        };

        struct Synchronization_resources
        {
            Synchronization_resources(std::size_t const count, Device const device) noexcept :
                device{device},
                available_frames_semaphores{create_semaphores(count, device, VK_SEMAPHORE_TYPE_BINARY)},
                finished_frames_semaphores{create_semaphores(count, device, VK_SEMAPHORE_TYPE_BINARY)},
                available_frames_fences{create_fences(count, device, VK_FENCE_CREATE_SIGNALED_BIT)}
            {
            }
            Synchronization_resources(Synchronization_resources const&) noexcept = delete;
            Synchronization_resources(Synchronization_resources&&) noexcept = delete;
            ~Synchronization_resources() noexcept
            {
                for (Fence const fence : available_frames_fences)
                {
                    destroy_fence(this->device, fence, {});
                }

                for (Semaphore const semaphore : finished_frames_semaphores)
                {
                    destroy_semaphore(this->device, semaphore, {});
                }

                for (Semaphore const semaphore : available_frames_semaphores)
                {
                    destroy_semaphore(this->device, semaphore, {});
                }
            }

            Synchronization_resources& operator=(Synchronization_resources const&) noexcept = delete;
            Synchronization_resources& operator=(Synchronization_resources&&) noexcept = delete;

            Device device;
            std::pmr::vector<Semaphore> available_frames_semaphores;
            std::pmr::vector<Semaphore> finished_frames_semaphores;
            std::pmr::vector<Fence> available_frames_fences;
        };

        Maia::Input::Keyboard_state get_keyboard_state() noexcept
        {
            Uint8 const* const sdl_keyboard_state = SDL_GetKeyboardState(nullptr);

            Maia::Input::Keyboard_state maia_keyboard_state;
            std::transform(sdl_keyboard_state, sdl_keyboard_state + 256, maia_keyboard_state.keys.begin(),
                [](Uint8 const state) -> bool { return state == 1; });
            
            return maia_keyboard_state;
        }

        Maia::Input::Mouse_state get_mouse_state() noexcept
        {
            int x = 0, y = 0;
            Uint32 const state = SDL_GetMouseState(&x, &y);

            return Maia::Input::Mouse_state
            {
                .position = {x, y},
                .keys = {
                    (state & SDL_BUTTON(SDL_BUTTON_LEFT)) != 0 ? true : false,
                    (state & SDL_BUTTON(SDL_BUTTON_MIDDLE)) != 0 ? true : false,
                    (state & SDL_BUTTON(SDL_BUTTON_RIGHT)) != 0 ? true : false,
                    (state & SDL_BUTTON(SDL_BUTTON_X1)) != 0 ? true : false,
                    (state & SDL_BUTTON(SDL_BUTTON_X2)) != 0 ? true : false,
                    false,
                    false,
                    false
                }
            };
        }

        struct Game_controller
        {
            Game_controller(int const joystick_index) noexcept :
                value{SDL_GameControllerOpen(joystick_index)}
            {
            }
            Game_controller(Game_controller const&) noexcept = delete;
            Game_controller(Game_controller&& other) noexcept :
                value{std::exchange(other.value, nullptr)}
            {
            }
            ~Game_controller() noexcept
            {
                if (this->value != nullptr)
                {
                    SDL_GameControllerClose(this->value);
                }
            }

            Game_controller& operator=(Game_controller const&) noexcept = delete;
            Game_controller& operator=(Game_controller&& other) noexcept
            {
                this->value = std::exchange(other.value, nullptr);

                return *this;
            }

            SDL_GameController* value;
        };

        Maia::Input::Game_controller_state get_game_controller_state(SDL_GameController& game_controller) noexcept
        {
            return {
                .left_axis = {
                    .x = SDL_GameControllerGetAxis(&game_controller, SDL_CONTROLLER_AXIS_LEFTX),
                    .y = SDL_GameControllerGetAxis(&game_controller, SDL_CONTROLLER_AXIS_LEFTY)
                },
                .right_axis = {
                    .x = SDL_GameControllerGetAxis(&game_controller, SDL_CONTROLLER_AXIS_RIGHTX),
                    .y = SDL_GameControllerGetAxis(&game_controller, SDL_CONTROLLER_AXIS_RIGHTY)
                },
                .left_trigger = {SDL_GameControllerGetAxis(&game_controller, SDL_CONTROLLER_AXIS_TRIGGERLEFT)},
                .right_trigger = {SDL_GameControllerGetAxis(&game_controller, SDL_CONTROLLER_AXIS_TRIGGERRIGHT)},
                .buttons = {
                    SDL_GameControllerGetButton(&game_controller, SDL_CONTROLLER_BUTTON_A) == 1,
                    SDL_GameControllerGetButton(&game_controller, SDL_CONTROLLER_BUTTON_B) == 1,
                    SDL_GameControllerGetButton(&game_controller, SDL_CONTROLLER_BUTTON_X) == 1,
                    SDL_GameControllerGetButton(&game_controller, SDL_CONTROLLER_BUTTON_Y) == 1,
                    SDL_GameControllerGetButton(&game_controller, SDL_CONTROLLER_BUTTON_BACK) == 1,
                    SDL_GameControllerGetButton(&game_controller, SDL_CONTROLLER_BUTTON_GUIDE) == 1,
                    SDL_GameControllerGetButton(&game_controller, SDL_CONTROLLER_BUTTON_START) == 1,
                    SDL_GameControllerGetButton(&game_controller, SDL_CONTROLLER_BUTTON_LEFTSTICK) == 1,
                    SDL_GameControllerGetButton(&game_controller, SDL_CONTROLLER_BUTTON_RIGHTSTICK) == 1,
                    SDL_GameControllerGetButton(&game_controller, SDL_CONTROLLER_BUTTON_LEFTSHOULDER) == 1,
                    SDL_GameControllerGetButton(&game_controller, SDL_CONTROLLER_BUTTON_RIGHTSHOULDER) == 1,
                    SDL_GameControllerGetButton(&game_controller, SDL_CONTROLLER_BUTTON_DPAD_UP) == 1,
                    SDL_GameControllerGetButton(&game_controller, SDL_CONTROLLER_BUTTON_DPAD_DOWN) == 1,
                    SDL_GameControllerGetButton(&game_controller, SDL_CONTROLLER_BUTTON_DPAD_LEFT) == 1,
                    SDL_GameControllerGetButton(&game_controller, SDL_CONTROLLER_BUTTON_DPAD_RIGHT) == 1
                }
            };
        }

        std::pmr::vector<Maia::Input::Game_controller_state> get_game_controllers_state(
            std::pmr::vector<Game_controller> const& game_controllers) noexcept
        {
            std::pmr::vector<Maia::Input::Game_controller_state> game_controllers_state;
            game_controllers_state.resize(game_controllers.size());
                
            for (std::size_t game_controller_index = 0; game_controller_index < game_controllers.size(); ++game_controller_index)
            {
                game_controllers_state[game_controller_index] = get_game_controller_state(*game_controllers[game_controller_index].value);
            }

            return game_controllers_state;
        }

        struct Application_resources
        {
            Application_resources(
                std::filesystem::path const shaders_path,
                VkPhysicalDevice const physical_device,
                Device const device,
                VkFormat const image_format) noexcept :
                device{device},
                pipeline_layout{create_pipeline_layout(device.value, empty_pipeline_layout_create_info())},
                render_pass{create_render_pass(device, image_format)},
                triangle_vertex_shader_module{create_shader_module(device, {}, convert_bytes<std::uint32_t>(read_bytes(shaders_path / "Triangle.vertex.spv")))},
                white_fragment_shader_module{create_shader_module(device, {}, convert_bytes<std::uint32_t>(read_bytes(shaders_path / "White.fragment.spv")))},
                white_triangle_pipeline{create_vertex_and_fragment_pipeline(device, {}, pipeline_layout, render_pass.value, 0, 1, triangle_vertex_shader_module.value, white_fragment_shader_module.value)}
            {
            }
            Application_resources(Application_resources const&) = delete;
            Application_resources(Application_resources&&) = delete;
            ~Application_resources() noexcept
            {
                if (white_triangle_pipeline != VK_NULL_HANDLE)
                {
                    vkDestroyPipeline(device.value, white_triangle_pipeline, nullptr);
                }

                if (triangle_vertex_shader_module.value != VK_NULL_HANDLE)
                {
                    destroy_shader_module(device, triangle_vertex_shader_module);
                }

                if (white_fragment_shader_module.value != VK_NULL_HANDLE)
                {
                    destroy_shader_module(device, white_fragment_shader_module);
                }

                if (render_pass.value != VK_NULL_HANDLE)
                {
                    destroy_render_pass(device, render_pass, {});
                }

                if (pipeline_layout != VK_NULL_HANDLE)
                {
                    destroy_pipeline_layout(device.value, pipeline_layout, {});
                }
            }

            Application_resources& operator=(Application_resources const&) = delete;
            Application_resources& operator=(Application_resources&&) = delete;

            Device device;
            VkPipelineLayout pipeline_layout;
            Render_pass render_pass;
            Shader_module triangle_vertex_shader_module;
            Shader_module white_fragment_shader_module;
            VkPipeline white_triangle_pipeline;
        };

        struct Device_memory_and_buffer
        {
            VkBuffer buffer;
            VkDeviceMemory memory;
            VkMemoryPropertyFlags memory_property_flags;
        };

        Device_memory_and_buffer create_device_memory_and_buffer(
            Physical_device_memory_properties const& physical_device_memory_properties,
            Device const device,
            VkDeviceSize const allocation_size, 
            VkBufferUsageFlags const usage,
            VkBufferCreateFlags const flags = {},
            VkSharingMode const sharing_mode = {},
            std::span<std::uint32_t const> const queue_family_indices = {},
            VkAllocationCallbacks const* const allocator = nullptr
        ) noexcept
        {
            VkBuffer const buffer = create_buffer(device.value, allocation_size, usage, flags, sharing_mode, queue_family_indices);

            Memory_requirements const memory_requirements = get_memory_requirements(device.value, buffer);
            Memory_type_bits const memory_type_bits = get_memory_type_bits(memory_requirements);

            std::optional<Memory_type_index_and_properties> const memory_type_index_and_properties = find_memory_type(
                physical_device_memory_properties,
                memory_type_bits,
                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
            );
            assert(memory_type_index_and_properties.has_value());

            VkDeviceMemory const device_memory =
                allocate_memory(device.value, memory_requirements.value.size, memory_type_index_and_properties->type_index, allocator);

            VkDeviceSize const device_offset = 0;
            bind_memory(device.value, buffer, device_memory, device_offset);

            VkMemoryPropertyFlags const memory_property_flags =
                physical_device_memory_properties.value.memoryTypes[memory_type_index_and_properties->type_index.value].propertyFlags;

            return {buffer, device_memory, memory_property_flags};
        }

        void destroy_device_memory_and_buffer(
            Device const device,
            Device_memory_and_buffer const device_memory_and_buffer,
            VkAllocationCallbacks const* const allocator = nullptr) noexcept
        {
            destroy_buffer(device.value, device_memory_and_buffer.buffer, allocator);
            free_memory(device.value, device_memory_and_buffer.memory, allocator);
        }

        void upload_data(
            Device const device,
            VkDeviceMemory const device_memory,
            VkDeviceSize const memory_offset,
            VkDeviceSize const memory_size,
            VkMemoryPropertyFlags const memory_property_flags,
            VkMemoryMapFlags const map_flags,
            std::function<void(void*)> const copy_data
        ) noexcept
        {
            assert(memory_property_flags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);

            {
                Mapped_memory const mapped_memory{device.value, device_memory, memory_offset, memory_size, map_flags};

                copy_data(mapped_memory.data());
            }

            if (!(memory_property_flags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT))
            {
                VkMappedMemoryRange const mapped_memory_range
                {
                    .sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE,
                    .pNext = nullptr,
                    .memory = device_memory,
                    .offset = memory_offset,
                    .size = memory_size,
                };

                check_result(
                    vkFlushMappedMemoryRanges(device.value, 1, &mapped_memory_range));
            }
        }

        void upload_data(
            VkMemoryPropertyFlags const memory_property_flags
        ) noexcept
        {
            assert(memory_property_flags & (VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT));

            if (memory_property_flags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)
            {
                // wrap this branch into a function
                // map
                // memcpy
                // unmap

                if (!(memory_property_flags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT))
                {
                    // vkFlushMappedMemoryRanges
                }
            }
            else
            {
                assert(memory_property_flags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

                // First check if there is available host visible buffer
                // If not, we need to create one

                // upload_data using host visible buffer

                // issue command to copy from host visible to device local
            }
        }

        Buffer_pool_memory_resource create_geometry_buffer_pool(
            VkPhysicalDeviceMemoryProperties const& physical_device_memory_properties,
            VkDevice const device
        ) noexcept
        {
            VkBufferCreateInfo const buffer_create_info
            {
                .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
                .pNext = nullptr,
                .flags = {},
                .size = 128*1024*1024,
                .usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
                .queueFamilyIndexCount = 0,
                .pQueueFamilyIndices = nullptr,
            };

            VkMemoryPropertyFlags constexpr memory_properties = 
                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;

            return 
            {
                physical_device_memory_properties,
                device,
                buffer_create_info,
                memory_properties,
                256
            };
        }

        void render(
            Command_buffer const command_buffer,
            VkPipeline const pipeline,
            VkRect2D const output_render_area,
            Mythology::ImGui::ImGui_resources const& imgui_resources
        ) noexcept
        {
            {
                vkCmdBindPipeline(command_buffer.value, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);

                {
                    std::array<VkViewport, 1> const viewports
                    {
                        VkViewport
                        {
                            .x = static_cast<float>(output_render_area.offset.x),
                            .y = static_cast<float>(output_render_area.offset.y),
                            .width = static_cast<float>(output_render_area.extent.width),
                            .height = static_cast<float>(output_render_area.extent.height),
                            .minDepth = 0.0f,
                            .maxDepth = 1.0f,
                        }
                    };
                    
                    vkCmdSetViewport(command_buffer.value, 0, static_cast<std::uint32_t>(viewports.size()), viewports.data());
                }

                {
                    std::array<VkRect2D, 1> const scissors
                    {
                        VkRect2D
                        {
                            .offset = output_render_area.offset,
                            .extent = output_render_area.extent,
                        }
                    };
                    
                    vkCmdSetScissor(command_buffer.value, 0, static_cast<std::uint32_t>(scissors.size()), scissors.data());
                }

                vkCmdDraw(command_buffer.value, 3, 1, 0, 0);

                {
                    ImDrawData const& draw_data = *::ImGui::GetDrawData();

                    Mythology::ImGui::render(
                        draw_data,
                        command_buffer.value,
                        imgui_resources.pipeline,
                        imgui_resources.pipeline_layout,
                        imgui_resources.descriptor_set,
                        {imgui_resources.buffer_pool->buffer(), imgui_resources.geometry_buffer_node.offset(), draw_data.TotalVtxCount * sizeof(ImDrawVert)},
                        {imgui_resources.buffer_pool->buffer(), imgui_resources.geometry_buffer_node.offset() + draw_data.TotalVtxCount * sizeof(ImDrawVert), draw_data.TotalIdxCount * sizeof(ImDrawIdx)}
                    );
                }
            }
        }

        void upload_buffer_data(
            VkDevice const device,
            VkDeviceMemory const device_memory,
            std::span<Buffer_pool_node const> const buffer_nodes,
            VkMemoryPropertyFlags const memory_properties,
            Maia::Utilities::glTF::Gltf const& gltf,
            std::filesystem::path const& gltf_directory
        ) noexcept
        {
            for (std::size_t buffer_index = 0; buffer_index < gltf.buffers.size(); ++buffer_index)
            {
                Maia::Utilities::glTF::Buffer const gltf_buffer = gltf.buffers[buffer_index];
                Buffer_pool_node const buffer_node = buffer_nodes[buffer_index];

                std::pmr::vector<std::byte> const buffer_data = Maia::Utilities::glTF::read_buffer_data(gltf_buffer, gltf_directory, {});

                auto const copy_buffer_data = [&buffer_data](void* const destination_data) -> void
                {
                    std::size_t const size_bytes = buffer_data.size() * sizeof(decltype(buffer_data)::value_type);
                    std::memcpy(destination_data, buffer_data.data(), size_bytes);
                };

                upload_data(
                    {device},
                    device_memory,
                    buffer_node.offset(),
                    buffer_node.size(),
                    memory_properties,
                    {},
                    copy_buffer_data);
            }
        }

        struct Material_type_id
        {
            std::uint32_t value;
        };

        struct Material_instance
        {
            Material_type_id type_id;
        };

        // TODO bounding box?
        struct Mesh_id
        {
            std::uint32_t value;
        };

        std::pmr::vector<Mesh_id> create_mesh_ids(
            Maia::Utilities::glTF::Gltf const& gltf,
            Mesh_id const first_mesh_id,
            std::pmr::polymorphic_allocator<> const& allocator
        ) noexcept
        {
            std::pmr::vector<Mesh_id> mesh_ids{allocator};
            mesh_ids.resize(gltf.meshes.size());

            for (std::uint32_t index = 0; index < gltf.meshes.size(); ++index)
            {
                mesh_ids[index] = {first_mesh_id.value + index};
            }

            return mesh_ids;
        }

        struct Primitive
        {
            struct Buffer_range
            {
                VkDeviceSize offset;
                VkDeviceSize size;
            };

            Mesh_id mesh_id;
            Material_instance material_instance;
            VkBuffer buffer;
            std::pmr::vector<Buffer_range> attribute_buffer_ranges;
        };

        std::size_t count_primitives(
            Maia::Utilities::glTF::Gltf const& gltf
        ) noexcept
        {
            std::size_t count = 0;

            for (Maia::Utilities::glTF::Mesh const& mesh : gltf.meshes)
            {
                count += mesh.primitives.size();
            }

            return count;
        }

        std::pmr::vector<Primitive> create_primitives(
            Maia::Utilities::glTF::Gltf const& gltf,
            std::span<Mesh_id const> const mesh_ids,
            std::span<VkBuffer const> const buffers,
            std::span<Material_instance const> const material_instances,
            Material_instance const default_material_instance,
            std::pmr::polymorphic_allocator<> const& allocator
        ) noexcept
        {
            using namespace Maia::Utilities::glTF;

            std::pmr::vector<Primitive> primitives{allocator};
            primitives.reserve(count_primitives(gltf));

            for (std::size_t mesh_index = 0; mesh_index < gltf.meshes.size(); ++mesh_index)
            {
                Mesh const& mesh = gltf.meshes[mesh_index];
                Mesh_id const mesh_id = mesh_ids[mesh_index];

                for (Maia::Utilities::glTF::Primitive const& primitive : mesh.primitives)
                {
                    // TODO assert that all attributes reference the same buffer

                    std::size_t const buffer_index = 
                        gltf.buffer_views[*gltf.accessors[primitive.attributes.at("position")].buffer_view_index].buffer_index;
                    VkBuffer const buffer = buffers[buffer_index];

                    std::pmr::vector<Primitive::Buffer_range> attribute_buffer_ranges{allocator};
                    attribute_buffer_ranges.reserve(primitive.attributes.size());

                    for (std::pair<std::pmr::string, std::size_t> const& attribute : primitive.attributes)
                    {
                        Accessor const& accessor = gltf.accessors[attribute.second];
                        // TODO accessor stride, offset, count
                        Buffer_view const& buffer_view = gltf.buffer_views[*accessor.buffer_view_index];

                        attribute_buffer_ranges.push_back(
                            {
                                .offset = buffer_view.byte_offset,
                                .size = buffer_view.byte_length,
                            }
                        );
                    }

                    Material_instance const material_instance = 
                        primitive.material_index.has_value() ?
                        material_instances[*primitive.material_index] :
                        default_material_instance;

                    primitives.push_back(
                        {
                            .mesh_id = mesh_id,
                            .material_instance = material_instance,
                            .buffer = buffer,
                            .attribute_buffer_ranges = std::move(attribute_buffer_ranges)
                        }
                    );
                }
            }

            return primitives;
        }
    }

    void run(
        nlohmann::json const& pipeline_json,
        std::filesystem::path const& pipeline_json_parent_path,
        std::filesystem::path const& gltf_file_path
    ) noexcept
    {
        std::filesystem::path const shaders_path = std::filesystem::current_path() / "../shaders";

        SDL_application sdl_application;
        
        if (SDL_Init(SDL_INIT_GAMECONTROLLER | SDL_INIT_VIDEO) != 0)
        {
            std::cerr << "Failed to initialize SDL: " << SDL_GetError() << '\n';
            std::exit(EXIT_FAILURE);
        }

        SDL_window const window
        {
            "Mythology",
            SDL_WINDOWPOS_UNDEFINED,
            SDL_WINDOWPOS_UNDEFINED,
            800,
            600,
            SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE | SDL_WINDOW_VULKAN
        };

        if (window.get() == nullptr)
        {
            std::cerr << "Could not create window: " << SDL_GetError() << '\n';
            std::exit(EXIT_FAILURE);
        }

        {
            using namespace std::chrono_literals;
            std::this_thread::sleep_for(5s);
        }

        Device_resources const device_resources{make_api_version(1, 2, 0), *window.get()};
        VkPhysicalDevice const physical_device = device_resources.physical_device;
        Device const device = device_resources.device;
        Queue_family_index const graphics_queue_family_index = device_resources.graphics_queue_family_index;
        Queue_family_index const present_queue_family_index = device_resources.present_queue_family_index;
        Surface const surface = device_resources.surface;

        using namespace Mythology::Core::Vulkan;

        std::size_t constexpr pipeline_length = 3;
        Synchronization_resources synchronization_resources{pipeline_length, device};
        std::span<Semaphore> const available_frames_semaphores = synchronization_resources.available_frames_semaphores;
        std::span<Semaphore> const finished_frames_semaphores = synchronization_resources.finished_frames_semaphores;
        std::span<Fence> const available_frames_fences = synchronization_resources.available_frames_fences;

        Queue const graphics_queue = get_device_queue(device, graphics_queue_family_index, 0);
        Queue const present_queue = get_device_queue(device, present_queue_family_index, 0);

        Command_pools_resources const command_pool_resources{device, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT, graphics_queue_family_index};
        Command_pool const command_pool = command_pool_resources.command_pool;
        std::pmr::vector<Command_buffer> const command_buffers = 
                allocate_command_buffers(
                    device,
                    command_pool,
                    VK_COMMAND_BUFFER_LEVEL_PRIMARY,
                    pipeline_length,
                    {}
                );

        enum class Game_key : std::uint8_t
        {
            Fire = 0,
            Jump,
            Fullscreen_toggle,
            Unused = 255
        };

        enum class Game_axis : std::uint8_t
        {
            Move_backwards = 0,
            Move_right,
            Unused = 255
        };

        enum class Game_trigger : std::uint8_t
        {
            Gradual_0 = 0,
            Gradual_1,
            Unused = 255
        };

        Maia::Input::Keyboard_mapping<9> constexpr keyboard_mapping = Maia::Input::create_keyboard_mapping<9>(
            {
                {{SDL_SCANCODE_F1}, Maia::Input::Key_id{static_cast<std::uint8_t>(Game_key::Fullscreen_toggle)}},
                {{SDL_SCANCODE_RETURN}, Maia::Input::Key_id{static_cast<std::uint8_t>(Game_key::Fire)}},
                {{SDL_SCANCODE_SPACE}, Maia::Input::Key_id{static_cast<std::uint8_t>(Game_key::Jump)}},
                {{SDL_SCANCODE_W}, Maia::Input::Negative_axis_id{static_cast<std::uint8_t>(Game_axis::Move_backwards)}},
                {{SDL_SCANCODE_S}, Maia::Input::Positive_axis_id{static_cast<std::uint8_t>(Game_axis::Move_backwards)}},
                {{SDL_SCANCODE_A}, Maia::Input::Negative_axis_id{static_cast<std::uint8_t>(Game_axis::Move_right)}},
                {{SDL_SCANCODE_D}, Maia::Input::Positive_axis_id{static_cast<std::uint8_t>(Game_axis::Move_right)}},
                {{SDL_SCANCODE_LCTRL}, Maia::Input::Trigger_id{static_cast<std::uint8_t>(Game_trigger::Gradual_0)}},
                {{SDL_SCANCODE_RCTRL}, Maia::Input::Trigger_id{static_cast<std::uint8_t>(Game_trigger::Gradual_1)}}
            }
        );

        Maia::Input::Mouse_mapping constexpr mouse_mapping
        {
            .keys = Maia::Input::create_array_mapping<8, Maia::Input::Mouse_key, Maia::Input::Key_id>({
                {{0}, {static_cast<std::uint8_t>(Game_key::Fire)}},
                {{2}, {static_cast<std::uint8_t>(Game_key::Jump)}}
            }),
            .horizontal_axis = Maia::Input::Axis_id{static_cast<std::uint8_t>(Game_axis::Move_right)},
            .vertical_axis = Maia::Input::Axis_id{static_cast<std::uint8_t>(Game_axis::Move_backwards)}
        };

        Maia::Input::Game_controller_mapping constexpr game_controller_mapping
        {
            .buttons = Maia::Input::create_array_mapping<15, Maia::Input::Game_controller_button, Maia::Input::Key_id>({
                {{SDL_CONTROLLER_BUTTON_A}, {static_cast<std::uint8_t>(Game_key::Fire)}},
                {{SDL_CONTROLLER_BUTTON_B}, {static_cast<std::uint8_t>(Game_key::Jump)}},
            }),
            .horizontal_left_axis = Maia::Input::Axis_id{static_cast<std::uint8_t>(Game_axis::Move_right)},
            .vertical_left_axis = Maia::Input::Axis_id{static_cast<std::uint8_t>(Game_axis::Move_backwards)},
            .left_trigger = Maia::Input::Trigger_id{static_cast<std::uint8_t>(Game_trigger::Gradual_0)},
            .right_trigger = Maia::Input::Trigger_id{static_cast<std::uint8_t>(Game_trigger::Gradual_1)}
        };

        std::pmr::vector<Maia::Input::Game_controller_mapping> const game_controllers_mappings{game_controller_mapping, game_controller_mapping};


        Maia::Input::Input_state previous_input_state{};

        std::pmr::vector<Game_controller> game_controllers;
        game_controllers.reserve(2);

        std::pmr::vector<VkRenderPass> const render_passes = 
            Maia::Renderer::Vulkan::create_render_passes(device.value, nullptr, pipeline_json.at("render_passes"), {}, {}, {}, {}, {}, {});

        std::pmr::vector<VkShaderModule> const shader_modules = 
            Maia::Renderer::Vulkan::create_shader_modules(device.value, nullptr, pipeline_json.at("shader_modules"), pipeline_json_parent_path, {});

        std::pmr::vector<VkSampler> const samplers = 
            Maia::Renderer::Vulkan::create_samplers(device.value, nullptr, pipeline_json.at("samplers"), {});
        
        std::pmr::vector<VkDescriptorSetLayout> const descriptor_set_layouts = 
            Maia::Renderer::Vulkan::create_descriptor_set_layouts(device.value, nullptr, samplers, pipeline_json.at("descriptor_set_layouts"), {});

        std::pmr::vector<VkPipelineLayout> const pipeline_layouts = 
            Maia::Renderer::Vulkan::create_pipeline_layouts(device.value, nullptr, descriptor_set_layouts, pipeline_json.at("pipeline_layouts"), {});

        std::pmr::vector<VkPipeline> const pipeline_states = create_pipeline_states(
            device.value,
            nullptr,
            shader_modules,
            pipeline_layouts,
            render_passes,
            pipeline_json.at("pipeline_states"),
            {}
        );

        Maia::Renderer::Vulkan::Commands_data const commands_data = Maia::Renderer::Vulkan::create_commands_data(
            pipeline_json.at("frame_commands"),
            pipeline_states,
            render_passes,
            {}
        );

        Maia::Utilities::glTF::Gltf const gltf = Maia::Utilities::glTF::gltf_from_json(read_json_from_file(gltf_file_path), {});
        Buffer_pool_memory_resource gltf_buffer_memory_resource = create_geometry_buffer_pool(get_phisical_device_memory_properties(physical_device).value, device.value);

        {
            std::pmr::vector<Buffer_pool_node> buffer_nodes;
            buffer_nodes.reserve(gltf.buffers.size());

            for (Maia::Utilities::glTF::Buffer const& gltf_buffer : gltf.buffers)
            {
                VkDeviceSize const bytes_to_allocate = gltf_buffer.byte_length;
                VkDeviceSize constexpr alignment = 0;
                VkBufferUsageFlags constexpr buffer_usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
                std::optional<Buffer_pool_node> const buffer_node = gltf_buffer_memory_resource.allocate(bytes_to_allocate, alignment, buffer_usage);
                assert(buffer_node.has_value());

                buffer_nodes.push_back(*buffer_node);
            }

            std::filesystem::path const gltf_directory = gltf_file_path.parent_path();
            VkDeviceMemory const device_memory = gltf_buffer_memory_resource.device_memory();
            VkMemoryPropertyFlags const memory_properties = gltf_buffer_memory_resource.memory_properties();

            upload_buffer_data(device.value, device_memory, buffer_nodes, memory_properties, gltf, gltf_directory);

            std::pmr::vector<Mesh_id> const mesh_ids = create_mesh_ids(gltf, Mesh_id{0}, {});
        }

        VkSurfaceFormatKHR const surface_format = select_surface_format(physical_device, surface);

        Application_resources const application_resources{shaders_path, physical_device, device, surface_format.format};
        Render_pass const render_pass = {render_passes[0]}; // TODO fix hack
        VkPipeline const white_triangle_pipeline = application_resources.white_triangle_pipeline;

        struct Clip_position
        {
            float x;
            float y;
        };

        std::array<Clip_position, 3> constexpr positions
        {
            {
                {0.0f, -0.5f},
                {-0.5f, 0.5f},
                {0.5f, 0.5f},
            }
        };

        VkDeviceSize constexpr positions_allocation_size = positions.size() * sizeof(decltype(positions)::value_type);

        Device_memory_and_buffer const positions_device_memory_and_buffer = create_device_memory_and_buffer(
            get_phisical_device_memory_properties(physical_device),
            device,
            positions_allocation_size,
            VkBufferUsageFlagBits::VK_BUFFER_USAGE_VERTEX_BUFFER_BIT
        );

        auto const copy_positions = [&positions](void* const destination_data) -> void
        {
            std::size_t const size_bytes = positions.size() * sizeof(decltype(positions)::value_type);
            std::memcpy(destination_data, positions.data(), size_bytes);
        };

        upload_data(
            device,
            positions_device_memory_and_buffer.memory,
            0,
            positions_allocation_size,
            positions_device_memory_and_buffer.memory_property_flags,
            {},
            copy_positions);


        std::array<VkDescriptorPoolSize, 2> constexpr descriptor_pool_sizes
        {
            {
                {VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1},
                {VK_DESCRIPTOR_TYPE_SAMPLER, 1}
            }
        };

        VkDescriptorPoolCreateInfo const descriptor_pool_create_info
        {
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
            .pNext = nullptr,
            .flags = {},
            .maxSets = 1,
            .poolSizeCount = static_cast<std::uint32_t>(descriptor_pool_sizes.size()),
            .pPoolSizes = descriptor_pool_sizes.data(),
        };

        VkDescriptorPool descriptor_pool = create_descriptor_pool(
            device.value,
            descriptor_pool_create_info
        );
        // TODO destroy descriptor pool

        Monotonic_device_memory_resource monotonic_memory_resource{device.value, 128*1024*1024};
        Buffer_pool_memory_resource geometry_buffer_memory_resource = create_geometry_buffer_pool(get_phisical_device_memory_properties(physical_device).value, device.value);

        Shader_module const imgui_vertex_shader_module = create_shader_module(device, {}, convert_bytes<std::uint32_t>(read_bytes(shaders_path / "Imgui.vertex.spv")));
        Shader_module const imgui_fragment_shader_module = create_shader_module(device, {}, convert_bytes<std::uint32_t>(read_bytes(shaders_path / "Imgui.fragment.spv")));
        
        ::ImGui::CreateContext();
        ::ImGui::StyleColorsDark();
        Mythology::ImGui::ImGui_resources imgui_resources
        {
            get_phisical_device_memory_properties(physical_device).value,
            device.value,
            descriptor_pool,
            render_pass.value,
            0,
            imgui_vertex_shader_module.value,
            imgui_fragment_shader_module.value,
            monotonic_memory_resource,
            geometry_buffer_memory_resource
        };

        {
            Command_buffer const command_buffer = command_buffers[0];
            reset_command_buffer(command_buffer, {});
            begin_command_buffer(command_buffer, VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT, {});

            Mythology::ImGui::upload_fonts_image_data(
                device.value,
                command_buffer.value,
                imgui_resources.fonts_image_resource.image,
                imgui_resources.fonts_image_resource.device_memory_range,
                imgui_resources.fonts_image_resource.memory_properties
            );

            end_command_buffer(command_buffer);

            queue_submit(graphics_queue, {}, {}, {&command_buffer, 1}, {}, {});

            vkDeviceWaitIdle(device.value);
        }
        
        destroy_shader_module(device, imgui_fragment_shader_module);
        destroy_shader_module(device, imgui_vertex_shader_module);

        Swapchain_resources swapchain_resources
        {
            physical_device,
            device,
            surface,
            surface_format,
            std::array<Queue_family_index, 2>{graphics_queue_family_index, present_queue_family_index},
            render_pass
        };
        
        Wait_device_idle_lock const wait_device_idle_lock{device};

        // ImGui state
        bool show_demo_window = true;
        bool show_another_window = false;
        ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

        Frame_index frame_index{0};
        bool isRunning = true;
        while (isRunning)
        {
            {
                SDL_Event event = {};
                while (SDL_PollEvent(&event))
                {
                    if (event.type == SDL_QUIT)
                    {
                        isRunning = false;
                        break;
                    }
                    else if (event.type == SDL_WINDOWEVENT)
                    {
                        if (event.window.event == SDL_WINDOWEVENT_SIZE_CHANGED || event.window.event == SDL_WINDOWEVENT_RESIZED)
                        {
                            vkDeviceWaitIdle(device.value);

                            Swapchain const old_swapchain = swapchain_resources.swapchain;
                            swapchain_resources = {physical_device, device, surface, surface_format, std::array<Queue_family_index, 2>{graphics_queue_family_index, present_queue_family_index}, render_pass, old_swapchain};
                        }
                    }
                    else if (event.type == SDL_CONTROLLERDEVICEADDED)
                    {
                        Sint32 const joystick_index = event.cdevice.which;
                        game_controllers.emplace_back(joystick_index);
                    }
                    else if (event.type == SDL_CONTROLLERDEVICEREMOVED)
                    {
                        Sint32 const removed_instance_id = event.cdevice.which;

                        auto const is_game_controller_with_id = [removed_instance_id](Game_controller const& game_controller) -> bool
                        {
                            SDL_Joystick* const joystick = SDL_GameControllerGetJoystick(game_controller.value);
                            SDL_JoystickID const instance_id = SDL_JoystickInstanceID(joystick);

                            return instance_id == removed_instance_id;
                        };

                        auto const game_controller = std::find_if(game_controllers.begin(), game_controllers.end(), is_game_controller_with_id);
                        if (game_controller != game_controllers.end())
                        {
                            game_controllers.erase(game_controller);
                        }
                    }
                }
            }

            if (isRunning)
            {
                Maia::Input::Keyboard_state const current_keyboard_state = get_keyboard_state();
                Maia::Input::Mouse_state const current_mouse_state = get_mouse_state();
                std::pmr::vector<Maia::Input::Game_controller_state> const current_game_controllers_state =
                    get_game_controllers_state(game_controllers);

                Maia::Input::Input_state const current_input_state = Maia::Input::map_input(current_game_controllers_state, game_controllers_mappings, current_keyboard_state, keyboard_mapping, current_mouse_state, mouse_mapping);

                if (Maia::Input::is_pressed({static_cast<std::uint8_t>(Game_key::Fullscreen_toggle)}, previous_input_state, current_input_state))
                {
                    toggle_fullscreen(*window.get());
                }

                if (Maia::Input::is_pressed({static_cast<std::uint8_t>(Game_key::Fire)}, previous_input_state, current_input_state))
                {
                    std::cout << "Fire!\n";
                }
                
                if (Maia::Input::is_pressed({static_cast<std::uint8_t>(Game_key::Jump)}, previous_input_state, current_input_state))
                {
                    std::cout << "Jump!\n";
                }
                
                if (current_input_state.axis[static_cast<std::uint8_t>(Game_axis::Move_backwards)].value != 0)
                {
                    std::cout << "Moving backwards: " << current_input_state.axis[static_cast<std::uint8_t>(Game_axis::Move_backwards)].normalized<float>() << "\n";
                }
                
                if (current_input_state.axis[static_cast<std::uint8_t>(Game_axis::Move_right)].value != 0)
                {
                    std::cout << "Moving right: " << current_input_state.axis[static_cast<std::uint8_t>(Game_axis::Move_right)].normalized<float>() << "\n";
                }

                if (current_input_state.triggers[static_cast<std::uint8_t>(Game_trigger::Gradual_0)].value != 0)
                {
                    std::cout << "Gradual 0: " << current_input_state.triggers[static_cast<std::uint8_t>(Game_trigger::Gradual_0)].normalized<float>() << "\n";
                }

                if (current_input_state.triggers[static_cast<std::uint8_t>(Game_trigger::Gradual_1)].value != 0)
                {
                    std::cout << "Gradual 1: " << current_input_state.triggers[static_cast<std::uint8_t>(Game_trigger::Gradual_1)].normalized<float>() << "\n";
                }

                {
                    ::ImGuiIO& io = ::ImGui::GetIO();
                    int w, h;
                    int display_w, display_h;
                    SDL_GetWindowSize(window.get(), &w, &h);
                    SDL_GL_GetDrawableSize(window.get(), &display_w, &display_h);
                    io.DisplaySize = ImVec2((float)w, (float)h);
                    if (w > 0 && h > 0)
                        io.DisplayFramebufferScale = ImVec2((float)display_w / w, (float)display_h / h);

                    // Setup time step (we don't use SDL_GetTicks() because it is using millisecond resolution)
                    static std::uint64_t g_Time = 0;
                    static std::uint64_t frequency = SDL_GetPerformanceFrequency();
                    std::uint64_t current_time = SDL_GetPerformanceCounter();
                    io.DeltaTime = g_Time > 0 ? (float)((double)(current_time - g_Time) / frequency) : (float)(1.0f / 60.0f);
                    g_Time = current_time;
                }

                ::ImGui::NewFrame();
                {
                    static float f = 0.0f;
                    static int counter = 0;

                    ::ImGui::Begin("Hello, world!");                          // Create a window called "Hello, world!" and append into it.

                    ::ImGui::Text("This is some useful text.");               // Display some text (you can use a format strings too)
                    ::ImGui::Checkbox("Demo Window", &show_demo_window);      // Edit bools storing our window open/close state
                    ::ImGui::Checkbox("Another Window", &show_another_window);

                    ::ImGui::SliderFloat("float", &f, 0.0f, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f
                    ::ImGui::ColorEdit3("clear color", (float*)&clear_color); // Edit 3 floats representing a color

                    if (::ImGui::Button("Button"))                            // Buttons return true when clicked (most widgets return true when edited/activated)
                        counter++;
                    ::ImGui::SameLine();
                    ::ImGui::Text("counter = %d", counter);

                    ::ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ::ImGui::GetIO().Framerate, ::ImGui::GetIO().Framerate);
                    ::ImGui::End();
                }
                ::ImGui::Render();

                {
                    Fence const available_frames_fence = available_frames_fences[frame_index.value];

                    if (is_fence_signaled(device, available_frames_fence))
                    {
                        Semaphore const available_frame_semaphore = available_frames_semaphores[frame_index.value];
                        std::optional<Swapchain_image_index> const swapchain_image_index =
                            acquire_next_image(device, swapchain_resources.swapchain, 0, available_frame_semaphore, {});

                        if (swapchain_image_index)
                        {
                            Image const swapchain_image = {swapchain_resources.images[swapchain_image_index->value]};
                            Framebuffer const swapchain_framebuffer = swapchain_resources.framebuffers[swapchain_image_index->value];

                            Command_buffer const command_buffer = command_buffers[frame_index.value];
                            reset_command_buffer(command_buffer, {});
                            begin_command_buffer(command_buffer, VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT, {});
                            {
                                {
                                    ImDrawData const& draw_data = *::ImGui::GetDrawData();

                                    imgui_resources.geometry_buffer_node = Mythology::ImGui::update_geometry_buffer(
                                        device.value,
                                        draw_data,
                                        imgui_resources.geometry_buffer_node,
                                        *imgui_resources.buffer_pool
                                    );
                                }

                                {
                                    VkClearColorValue const clear_color
                                    {
                                        .float32 = {0.0f, 0.0f, 1.0f, 1.0f}
                                    };

                                    VkRect2D const output_render_area
                                    {
                                        .offset = {0, 0},
                                        .extent = swapchain_resources.extent
                                    };

                                    VkImageSubresourceRange const output_image_subresource_range
                                    {
                                        .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT, 
                                        .baseMipLevel = 0,
                                        .levelCount = 1, 
                                        .baseArrayLayer = 0,
                                        .layerCount = 1
                                    };

                                    Maia::Renderer::Vulkan::draw(
                                        command_buffer.value,
                                        swapchain_image.value,
                                        output_image_subresource_range,
                                        swapchain_framebuffer.value,
                                        output_render_area,
                                        commands_data
                                    );

                                    /*clear_and_begin_render_pass(command_buffer, render_pass, swapchain_framebuffer, clear_color, swapchain_image, output_image_subresource_range, output_render_area);
                                    render(command_buffer, white_triangle_pipeline, output_render_area, imgui_resources);
                                    end_render_pass_and_switch_layout(command_buffer, swapchain_image, output_image_subresource_range, true);*/
                                }
                            }
                            end_command_buffer(command_buffer);

                            Semaphore const finished_frame_semaphore = finished_frames_semaphores[frame_index.value];
                            {
                                std::array<VkPipelineStageFlags, 1> constexpr wait_destination_stage_masks = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
                                reset_fences(device, {&available_frames_fence, 1});
                                queue_submit(graphics_queue, {&available_frame_semaphore, 1}, wait_destination_stage_masks, {&command_buffer, 1}, {&finished_frame_semaphore, 1}, available_frames_fence);
                            }

                            queue_present(present_queue, {&finished_frame_semaphore.value, 1}, swapchain_resources.swapchain, *swapchain_image_index);

                            frame_index.value = (frame_index.value + 1) % pipeline_length;
                        }
                    }
                }

                previous_input_state = current_input_state;
            }
        }
    }
}