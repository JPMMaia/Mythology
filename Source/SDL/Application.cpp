module mythology.sdl.application;

import maia.input;
import maia.renderer.vulkan;
import maia.sdl.vulkan;
import mythology.core.vulkan;

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

namespace Mythology::SDL
{
    namespace
    {
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
                m_window = std::exchange(other.m_window, nullptr);
                
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
            Physical_device const physical_device,
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
            Physical_device const physical_device,
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
            Physical_device const physical_device,
            Device const device,
            Surface const surface,
            std::span<Queue_family_index const> const queue_family_indices,
            std::optional<Swapchain> const old_swapchain = {}
        ) noexcept
        {
            assert(!queue_family_indices.empty());

            VkSurfaceCapabilitiesKHR const surface_capabilities = get_surface_capabilities(physical_device, surface);
            std::uint32_t const image_count = select_swapchain_image_count(surface_capabilities);
            VkSurfaceFormatKHR const selected_surface_format = select_surface_format(physical_device, surface);
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
                selected_surface_format.format,
                selected_surface_format.colorSpace,
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
                
                this->surface = {create_surface(window, this->instance.value)};
                
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

                if (this->instance.value != VK_NULL_HANDLE)
                {
                    destroy_instance(this->instance);
                }
            }

            Device_resources& operator=(Device_resources const&) = delete;
            Device_resources& operator=(Device_resources&&) = delete;

            Instance instance = {};
            Physical_device physical_device = {};
            Surface surface = {};
            Queue_family_index graphics_queue_family_index = {};
            Queue_family_index present_queue_family_index = {};
            Device device = {};
        };

        struct Swapchain_raii
        {
            Swapchain_raii(
                Physical_device const physical_device,
                Device const device,
                Surface const surface,
                std::span<Queue_family_index const> const queue_family_indices,
                std::optional<Swapchain> const old_swapchain = {}) noexcept :
                device{device},
                swapchain{create_swapchain(physical_device, device, surface, queue_family_indices, old_swapchain)}
            {
            }
            Swapchain_raii(Swapchain_raii const&) = delete;
            Swapchain_raii(Swapchain_raii&& other) noexcept :
                device{std::exchange(other.device, {VK_NULL_HANDLE})},
                swapchain{std::exchange(other.swapchain, {VK_NULL_HANDLE})}
            {
            }
            ~Swapchain_raii() noexcept
            {
                if (this->swapchain.value != VK_NULL_HANDLE)
                {
                    destroy_swapchain(this->device, this->swapchain);
                }
            }

            Swapchain_raii& operator=(Swapchain_raii const&) = delete;
            Swapchain_raii& operator=(Swapchain_raii&& other) noexcept
            {
                this->device = std::exchange(other.device, {VK_NULL_HANDLE});
                this->swapchain = std::exchange(other.swapchain, {VK_NULL_HANDLE});

                return *this;
            }

            Device device = {};
            Swapchain swapchain = {};
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

        std::pmr::vector<std::byte> read_bytes(std::filesystem::path const& file_path) noexcept
        {
            std::ifstream input_stream{file_path, std::ios::in | std::ios::binary};
            assert(input_stream.good());

            input_stream.seekg(0, std::ios::end);
            auto const size_in_bytes = input_stream.tellg();

            std::pmr::vector<std::byte> buffer;
            buffer.resize(size_in_bytes);

            input_stream.seekg(0, std::ios::beg);
            input_stream.read(reinterpret_cast<char*>(buffer.data()), buffer.size());

            return buffer;
        }

        template<typename Value_type>
        std::pmr::vector<Value_type> convert_bytes(std::span<std::byte const> const bytes) noexcept
        {
            assert(bytes.size_bytes() % sizeof(Value_type) == 0);

            std::pmr::vector<Value_type> values;
            values.resize(bytes.size_bytes() / sizeof(Value_type));

            std::memcpy(values.data(), bytes.data(), bytes.size_bytes());

            return values;
        }
    }

    void run() noexcept
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

        Device_resources const device_resources{make_api_version(1, 2, 0), *window.get()};
        Device const device = device_resources.device;
        Queue_family_index const graphics_queue_family_index = device_resources.graphics_queue_family_index;
        Queue_family_index const present_queue_family_index = device_resources.present_queue_family_index;

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

        Swapchain_raii swapchain_raii{device_resources.physical_device, device, device_resources.surface, std::array<Queue_family_index, 2>{graphics_queue_family_index, present_queue_family_index}};
        std::pmr::vector<VkImage> swapchain_images = get_swapchain_images(device, swapchain_raii.swapchain);
        //std::pmr::vector<VkImageView> const swapchain_image_views = create_swapchain_image_views(device, swapchain_images, select_surface_format(physical_device, surface).format);

        /*VkExtent3D constexpr color_image_extent{16, 16, 1};
        Device_memory_and_color_image const device_memory_and_color_image = 
            create_device_memory_and_color_image(physical_device, device, VK_FORMAT_R8G8B8A8_UINT, color_image_extent);*/

        Maia::Renderer::Vulkan::Shader_module const triangle_vertex_shader_module = create_shader_module(device, {}, convert_bytes<std::uint32_t>(read_bytes(shaders_path / "Triangle.vertex.spv")));
        Maia::Renderer::Vulkan::Shader_module const white_fragment_shader_module = create_shader_module(device, {}, convert_bytes<std::uint32_t>(read_bytes(shaders_path / "White.fragment.spv")));

        Wait_for_all_fences_lock const wait_for_all_fences_lock{device, available_frames_fences, Timeout_nanoseconds{5000000000}};
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
                            Swapchain const old_swapchain = swapchain_raii.swapchain;
                            swapchain_raii = {device_resources.physical_device, device, device_resources.surface, std::array<Queue_family_index, 2>{graphics_queue_family_index, present_queue_family_index}, old_swapchain};
                            swapchain_images = get_swapchain_images(device, swapchain_raii.swapchain);
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
                    Fence const available_frames_fence = available_frames_fences[frame_index.value];

                    if (is_fence_signaled(device, available_frames_fence))
                    {
                        Semaphore const available_frame_semaphore = available_frames_semaphores[frame_index.value];
                        std::optional<Swapchain_image_index> const swapchain_image_index =
                            acquire_next_image(device, swapchain_raii.swapchain, 0, available_frame_semaphore, {});

                        if (swapchain_image_index)
                        {
                            Image const swapchain_image = {swapchain_images[swapchain_image_index->value]};

                            Command_buffer const command_buffer = command_buffers[frame_index.value];
                            reset_command_buffer(command_buffer, {});
                            begin_command_buffer(command_buffer, VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT, {});
                            {
                                VkClearColorValue const clear_color = {
                                    .float32 = {0.0f, 0.0f, 1.0f, 1.0f}
                                };

                                render(command_buffer, swapchain_image, clear_color, true);
                            }
                            end_command_buffer(command_buffer);

                            Semaphore const finished_frame_semaphore = finished_frames_semaphores[frame_index.value];
                            {
                                std::array<VkPipelineStageFlags, 1> constexpr wait_destination_stage_masks = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
                                reset_fences(device, {&available_frames_fence, 1});
                                queue_submit(graphics_queue, {&available_frame_semaphore, 1}, wait_destination_stage_masks, {&command_buffer, 1}, {&finished_frame_semaphore, 1}, available_frames_fence);
                            }

                            queue_present(present_queue, {&finished_frame_semaphore.value, 1}, swapchain_raii.swapchain, *swapchain_image_index);

                            frame_index.value = (frame_index.value + 1) % pipeline_length;
                        }
                    }
                }

                previous_input_state = current_input_state;
            }
        }
    }
}