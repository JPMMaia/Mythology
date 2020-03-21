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
            std::span<Queue_family_index const> const queue_family_indices
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
                present_mode
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
                    this->swapchain = create_swapchain(this->physical_device, this->device, this->surface, queue_family_indices);
                }
                else
                {
                    Queue_family_index const queue_family_index = this->present_queue_family_index;
                    this->device = create_device(this->physical_device, {&queue_family_index, 1}, is_extension_to_enable);
                    this->swapchain = create_swapchain(this->physical_device, this->device, this->surface, {&queue_family_index, 1});
                }
            }
            Device_resources(Device_resources const&) = delete;
            Device_resources(Device_resources&&) = delete;
            ~Device_resources() noexcept
            {
                if (this->swapchain.value != VK_NULL_HANDLE)
                {
                    destroy_swapchain(this->device, this->swapchain);
                }

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

        
        struct Input_key
        {
            std::uint8_t value;

            static Input_key invalid_value() noexcept
            {
                return {std::numeric_limits<std::uint8_t>::max()};
            }

            bool is_valid() const noexcept
            {
                return this->value != invalid_value().value;
            }
        };

        struct Input_axis
        {
            std::uint8_t value;

            static Input_axis invalid_value() noexcept
            {
                return {std::numeric_limits<std::uint8_t>::max()};
            }

            bool is_valid() const noexcept
            {
                return this->value != invalid_value().value;
            }
        };

        struct Input_trigger
        {
            std::uint8_t value;

            static Input_trigger invalid_value() noexcept
            {
                return {std::numeric_limits<std::uint8_t>::max()};
            }

            bool is_valid() const noexcept
            {
                return this->value != invalid_value().value;
            }
        };

        struct Input_state
        {
            std::array<bool, 256> keys;
            std::array<std::int16_t, 8> axis;
            std::array<std::int16_t, 2> triggers;
        };

        struct Input_positive_axis
        {
            std::uint8_t value;
        };

        struct Input_negative_axis
        {
            std::uint8_t value;
        };

        template <std::size_t Count>
        struct Keyboard_mapping
        {
            using Key_type = Maia::Input::Keyboard_key;
            using Value_type = std::variant<Input_key, Input_positive_axis, Input_negative_axis, Input_trigger>;

            std::array<Key_type, Count> keys;
            std::array<Value_type, Count> values;
        };

        template <std::size_t Count>
        using Keyboard_mapping_key_value_pair = std::pair<typename Keyboard_mapping<Count>::Key_type, typename Keyboard_mapping<Count>::Value_type>;

        template <std::size_t Count>
        Keyboard_mapping<Count> create_keyboard_mapping(
            std::initializer_list<Keyboard_mapping_key_value_pair<Count>> const key_value_pairs) noexcept
        {
            Keyboard_mapping<Count> keyboard_mapping{};

            for (auto key_value_it = key_value_pairs.begin(); key_value_it != key_value_pairs.end(); ++key_value_it)
            {
                auto const key_index = std::distance(key_value_pairs.begin(), key_value_it);

                keyboard_mapping.keys[key_index] = key_value_it->first;
                keyboard_mapping.values[key_index] = key_value_it->second;
            }

            return keyboard_mapping;
        }

        struct Mouse_mapping
        {
            std::array<Input_key, 8> keys;
            Input_axis horizontal_axis;
            Input_axis vertical_axis;
        };

        Mouse_mapping create_mouse_mapping(
            std::initializer_list<std::pair<Maia::Input::Mouse_key, Input_key>> const key_value_pairs,
            std::optional<Input_axis> horizontal_axis,
            std::optional<Input_axis> vertical_axis) noexcept
        {
            Mouse_mapping mouse_mapping{};

            for (std::size_t key_index = 0; key_index < mouse_mapping.keys.size(); ++key_index)
            {
                mouse_mapping.keys[key_index] = Input_key::invalid_value();
            }

            for (std::pair<Maia::Input::Mouse_key, Input_key> const key_value : key_value_pairs)
            {
                mouse_mapping.keys[key_value.first.value] = key_value.second;
            }

            mouse_mapping.horizontal_axis = horizontal_axis->is_valid() ? *horizontal_axis : Input_axis::invalid_value();
            mouse_mapping.vertical_axis = vertical_axis->is_valid() ? *vertical_axis : Input_axis::invalid_value();

            return mouse_mapping;
        }

        struct Game_controller_mapping
        {
            Input_axis horizontal_left_axis;
            Input_axis vertical_left_axis;
            Input_axis horizontal_right_axis;
            Input_axis vertical_right_axis;
            Input_trigger left_trigger;
            Input_trigger right_trigger;
            std::array<Input_key, 15> keys;
        };

        /*struct Input_mapping
        {
            Keyboard_mapping keyboard;
            Mouse_mapping mouse;
            std::array<Game_controller_mapping, 2> game_controllers;
        };

        Input_mapping create_input_mapping() noexcept
        {

        }*/

        template <std::size_t Keyboard_count>
        Input_state map_input(
            std::span<Maia::Input::Game_controller_state> const& game_controller_states,
            std::span<Game_controller_mapping> const& game_controller_mappings,
            Maia::Input::Keyboard_state const& keyboard_state,
            Keyboard_mapping<Keyboard_count> const& keyboard_mapping,
            Maia::Input::Mouse_state const& mouse_state,
            Mouse_mapping const& mouse_mapping) noexcept
        {
            assert(game_controller_states.size() == game_controller_mappings.size());

            Input_state input_state{};

            for (std::size_t key_index = 0; key_index < Keyboard_count; ++key_index)
            {
                Maia::Input::Keyboard_key const key = keyboard_mapping.keys[key_index];
                bool const key_down = keyboard_state.keys[key.value];

                if (key_down)
                {
                    std::variant<Input_key, Input_positive_axis, Input_negative_axis, Input_trigger> const input_element = keyboard_mapping.values[key_index];

                    if (std::holds_alternative<Input_key>(input_element))
                    {
                        Input_key const input_key = std::get<Input_key>(input_element);
                        input_state.keys[input_key.value] |= key_down;
                    }
                    else if (std::holds_alternative<Input_positive_axis>(input_element))
                    {
                        Input_positive_axis const input_positive_axis = std::get<Input_positive_axis>(input_element);
                        input_state.axis[input_positive_axis.value] = std::numeric_limits<std::int16_t>::max();
                    }
                    else if (std::holds_alternative<Input_negative_axis>(input_element))
                    {
                        Input_negative_axis const input_negative_axis = std::get<Input_negative_axis>(input_element);
                        input_state.axis[input_negative_axis.value] = std::numeric_limits<std::int16_t>::lowest();
                    }
                    else if (std::holds_alternative<Input_trigger>(input_element))
                    {
                        Input_trigger const input_trigger = std::get<Input_trigger>(input_element);
                        input_state.triggers[input_trigger.value] = std::numeric_limits<std::int16_t>::max();
                    }
                }
            }

            {
                // static_assert(mouse_mapping.keys.size() == mouse_state.keys.size());

                for (std::size_t key_index = 0; key_index < mouse_mapping.keys.size(); ++key_index)
                {
                    bool const key_down = mouse_state.keys[key_index];

                    if (key_down)
                    {
                        Input_key const input_key = mouse_mapping.keys[key_index];
                        input_state.keys[input_key.value] |= key_down;
                    }
                }

                if (mouse_mapping.horizontal_axis.is_valid())
                {
                    // TODO
                }

                if (mouse_mapping.vertical_axis.is_valid())
                {
                    // TODO
                }
            }

            return input_state;
        }
    }

    void run() noexcept
    {
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
            SDL_WINDOW_SHOWN | SDL_WINDOW_VULKAN
        };

        if (window.get() == nullptr)
        {
            std::cerr << "Could not create window: " << SDL_GetError() << '\n';
            std::exit(EXIT_FAILURE);
        }

        Device_resources const device_resources{make_api_version(1, 2, 0), *window.get()};
        Device const device = device_resources.device;
        Swapchain const swapchain = device_resources.swapchain;
        Queue_family_index const graphics_queue_family_index = device_resources.graphics_queue_family_index;
        Queue_family_index const present_queue_family_index = device_resources.present_queue_family_index;

        using namespace Mythology::Core::Vulkan;

        std::pmr::vector<VkImage> const swapchain_images = get_swapchain_images(device, swapchain);
        //std::pmr::vector<VkImageView> const swapchain_image_views = create_swapchain_image_views(device, swapchain_images, select_surface_format(physical_device, surface).format);

        /*VkExtent3D constexpr color_image_extent{16, 16, 1};
        Device_memory_and_color_image const device_memory_and_color_image = 
            create_device_memory_and_color_image(physical_device, device, VK_FORMAT_R8G8B8A8_UINT, color_image_extent);*/

        std::size_t const pipeline_length = swapchain_images.size();
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

        Maia::Input::Keyboard_state previous_keyboard_state{};
        Maia::Input::Mouse_state previous_mouse_state{};

        Wait_for_all_fences_lock const wait_for_all_fences_lock{device, available_frames_fences, Timeout_nanoseconds{5000000000}};

        std::pmr::vector<Game_controller> game_controllers;
        game_controllers.reserve(2);

        


        // ----------

        enum class Game_key : std::uint8_t
        {
            Fire = 0,
            Jump,
            Invalid = 255
        };

        enum class Game_axis : std::uint8_t
        {
            Move_forward = 0,
            Move_right,
            Invalid = 255
        };

        enum class Game_trigger : std::uint8_t
        {
            Gradual_0 = 0,
            Gradual_1,
            Invalid = 255
        };

        Keyboard_mapping<8> const keyboard_mapping = create_keyboard_mapping<8>(
            {
                {{SDL_SCANCODE_RETURN}, Input_key{static_cast<std::uint8_t>(Game_key::Fire)}},
                {{SDL_SCANCODE_SPACE}, Input_key{static_cast<std::uint8_t>(Game_key::Jump)}},
                {{SDL_SCANCODE_W}, Input_positive_axis{static_cast<std::uint8_t>(Game_axis::Move_forward)}},
                {{SDL_SCANCODE_S}, Input_negative_axis{static_cast<std::uint8_t>(Game_axis::Move_forward)}},
                {{SDL_SCANCODE_A}, Input_negative_axis{static_cast<std::uint8_t>(Game_axis::Move_right)}},
                {{SDL_SCANCODE_D}, Input_positive_axis{static_cast<std::uint8_t>(Game_axis::Move_right)}},
                {{SDL_SCANCODE_LCTRL}, Input_trigger{static_cast<std::uint8_t>(Game_trigger::Gradual_0)}},
                {{SDL_SCANCODE_RCTRL}, Input_trigger{static_cast<std::uint8_t>(Game_trigger::Gradual_1)}}
            }
        );

        Mouse_mapping const mouse_mapping = create_mouse_mapping(
            {
                {{0}, Input_key{static_cast<std::uint8_t>(Game_key::Fire)}},
                {{2}, Input_key{static_cast<std::uint8_t>(Game_key::Jump)}},
            },
            Input_axis{static_cast<std::int8_t>(Game_axis::Move_right)},
            Input_axis{static_cast<std::int8_t>(Game_axis::Move_forward)}
        );

        /*
        Input_mapping const input_mapping = create_input_mapping(
            {
                {.key = SDLK_RETURN}
            }
        );*/
        /*
        input_mapping.keyboard.keys[SDLK_RETURN] = {static_cast<std::uint8_t>(Game_key::Fire)};
        input_mapping.keyboard.keys[SDLK_SPACE] = {static_cast<std::uint8_t>(Game_key::Jump)};
        input_mapping.keyboard.axis[SDLK_w] = {static_cast<std::int8_t>(Game_axis::Move_forward)};
        input_mapping.keyboard.axis[SDLK_s] = {-static_cast<std::int8_t>(Game_axis::Move_forward)};
        input_mapping.keyboard.axis[SDLK_a] = {-static_cast<std::int8_t>(Game_axis::Move_right)};
        input_mapping.keyboard.axis[SDLK_d] = {static_cast<std::int8_t>(Game_axis::Move_right)};
        
        input_mapping.mouse.keys[SDL_BUTTON(1)] = {static_cast<std::uint8_t>(Game_key::Fire)};
        input_mapping.mouse.keys[SDL_BUTTON(3)] = {static_cast<std::uint8_t>(Game_key::Jump)};
        input_mapping.mouse.horizontal_axis = {static_cast<std::int8_t>(Game_axis::Move_right)};
        input_mapping.mouse.vertical_axis = {static_cast<std::int8_t>(Game_axis::Move_forward)};

        input_mapping.game_controllers[0].keys[SDL_CONTROLLER_BUTTON_A] = {static_cast<std::uint8_t>(Game_key::Fire)};
        input_mapping.game_controllers[0].keys[SDL_CONTROLLER_BUTTON_B] = {static_cast<std::uint8_t>(Game_key::Jump)};
        input_mapping.game_controllers[0].horizontal_left_axis = {static_cast<std::int8_t>(Game_axis::Move_right)};
        input_mapping.game_controllers[0].vertical_left_axis = {static_cast<std::int8_t>(Game_axis::Move_forward)};
        */

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
                std::pmr::vector<Maia::Input::Game_controller_state> const game_controllers_state =
                    get_game_controllers_state(game_controllers);

                Input_state const input_state = map_input({}, {}, current_keyboard_state, keyboard_mapping, current_mouse_state, mouse_mapping);

                if (input_state.keys[static_cast<std::uint8_t>(Game_key::Fire)])
                {
                    std::cout << "Fire!\n";
                }
                
                if (input_state.keys[static_cast<std::uint8_t>(Game_key::Jump)])
                {
                    std::cout << "Jump!\n";
                }
                
                if (input_state.axis[static_cast<std::uint8_t>(Game_axis::Move_forward)] != 0)
                {
                    std::cout << "Moving forward: " << input_state.axis[static_cast<std::uint8_t>(Game_axis::Move_forward)] << "\n";
                }
                
                if (input_state.axis[static_cast<std::uint8_t>(Game_axis::Move_right)] != 0)
                {
                    std::cout << "Moving right: " << input_state.axis[static_cast<std::uint8_t>(Game_axis::Move_right)] << "\n";
                }

                if (input_state.triggers[static_cast<std::uint8_t>(Game_trigger::Gradual_0)] != 0)
                {
                    std::cout << "Gradual 0: " << input_state.triggers[static_cast<std::uint8_t>(Game_trigger::Gradual_0)] << "\n";
                }

                if (input_state.triggers[static_cast<std::uint8_t>(Game_trigger::Gradual_1)] != 0)
                {
                    std::cout << "Gradual 1: " << input_state.triggers[static_cast<std::uint8_t>(Game_trigger::Gradual_1)] << "\n";
                }

                /*using Input_key = std::variant<Maia::Input::Keyboard_key>;
                std::array<Input_key, 256> virtual_to_input_key{};*/

                {
                    Fence const available_frames_fence = available_frames_fences[frame_index.value];

                    if (is_fence_signaled(device, available_frames_fence))
                    {
                        Semaphore const available_frame_semaphore = available_frames_semaphores[frame_index.value];
                        std::optional<Swapchain_image_index> const swapchain_image_index =
                            acquire_next_image(device, swapchain, 0, available_frame_semaphore, {});

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

                            queue_present(present_queue, {&finished_frame_semaphore.value, 1}, swapchain, *swapchain_image_index);

                            frame_index.value = (frame_index.value + 1) % pipeline_length;
                        }
                    }
                }

                previous_keyboard_state = current_keyboard_state;
                previous_mouse_state = current_mouse_state;
            }
        }
    }
}