export module maia.renderer.vulkan.memory_management;

import maia.renderer.vulkan.device_memory;

import <vulkan/vulkan.h>;

import <memory_resource>;
import <optional>;
import <unordered_map>;
import <vector>;

namespace Maia::Renderer::Vulkan
{
    export struct Device_memory_range
    {
        VkDeviceMemory device_memory;
        VkDeviceSize offset;
        VkDeviceSize size;
    };

    export Device_memory_range allocate_device_memory(
        VkDevice device,
        Memory_type_index memory_type_index,
        VkDeviceSize allocation_size,
        VkAllocationCallbacks const* vulkan_allocator = nullptr
    ) noexcept;

    export void free_device_memory(
        VkDevice device,
        Device_memory_range device_memory_range,
        VkAllocationCallbacks const* vulkan_allocator = nullptr
    ) noexcept;
    
    export class Monotonic_device_memory_resource
    {
    public:

        struct Chunk
        {
            Device_memory_range device_memory_range;
            VkDeviceSize allocated_bytes;
        };

        Monotonic_device_memory_resource(
            VkDevice device,
            VkDeviceSize blocks_size,
            VkAllocationCallbacks const* vulkan_allocator = nullptr,
            std::pmr::polymorphic_allocator<std::pair<Memory_type_index const, Chunk>> const& allocator = {}
        ) noexcept;
        Monotonic_device_memory_resource(Monotonic_device_memory_resource const&) noexcept = delete;
        Monotonic_device_memory_resource(Monotonic_device_memory_resource&& other) noexcept;
        ~Monotonic_device_memory_resource() noexcept;

        Monotonic_device_memory_resource& operator=(Monotonic_device_memory_resource const&) noexcept = delete;
        Monotonic_device_memory_resource& operator=(Monotonic_device_memory_resource&& other) noexcept;

        Device_memory_range allocate(
            Memory_type_index memory_type_index,
            VkDeviceSize bytes_to_allocate,
            VkDeviceSize alignment
        ) noexcept;

    private:

        std::pmr::unordered_multimap<Memory_type_index, Chunk> m_chunks;
        VkDevice m_device;
        VkDeviceSize m_blocks_size;
        VkAllocationCallbacks const* m_vulkan_allocator;
    };


    struct Tree_node_index
    {
        std::uint32_t value;
    };

    export struct Tree_level
    {
        std::uint8_t value;
    };

    export struct Tree_level_index
    {
        std::uint32_t value;
    };

    export struct Tree_node
    {
        Tree_level level;
        Tree_level_index level_index;
    };

    export using Memory_size = VkDeviceSize;

    export struct Memory_tree
    {
        std::pmr::vector<bool> allocated;
        std::pmr::vector<bool> internal;
        Memory_size minimum_block_size;
        Memory_size maximum_block_size;
    };

    export Memory_tree create_memory_tree(
        Memory_size minimum_block_size,
        Memory_size maximum_block_size,
        std::pmr::polymorphic_allocator<bool> const& allocator = {}
    ) noexcept;

    export std::optional<Tree_node> allocate_node(
        Memory_tree& tree,
        Memory_size required_size
    ) noexcept;

    export void free_node(
        Memory_tree& tree,
        Tree_node const node_to_free
    ) noexcept;


    export struct Node_device_memory_resource
    {
        Device_memory_range device_memory_range;
        Memory_tree& memory_tree;
        Tree_node node;
    };

    export class Pool_device_memory_resource
    {
    public:

        struct Chunk
        {
            Device_memory_range device_memory_range;
            Memory_tree memory_tree;
        };

        Pool_device_memory_resource(
            VkDevice device,
            VkDeviceSize maximum_block_size,
            VkDeviceSize minimum_block_size,
            VkAllocationCallbacks const* vulkan_allocator = nullptr,
            std::pmr::polymorphic_allocator<std::pair<Memory_type_index const, Chunk>> const& chunk_allocator = {},
            std::pmr::polymorphic_allocator<bool> bool_allocator = {}
        ) noexcept;
        Pool_device_memory_resource(Pool_device_memory_resource const&) noexcept = delete;
        Pool_device_memory_resource(Pool_device_memory_resource&& other) noexcept;
        ~Pool_device_memory_resource() noexcept;

        Pool_device_memory_resource& operator=(Pool_device_memory_resource const&) noexcept = delete;
        Pool_device_memory_resource& operator=(Pool_device_memory_resource&& other) noexcept;

        Node_device_memory_resource allocate(
            Memory_type_index memory_type_index,
            VkDeviceSize bytes_to_allocate,
            VkDeviceSize alignment
        ) noexcept;

        void deallocate(
            Node_device_memory_resource memory_resource
        ) noexcept;

    private:

        std::pmr::unordered_multimap<Memory_type_index, Chunk> m_chunks;
        VkDevice m_device;
        VkDeviceSize m_maximum_block_size;
        VkDeviceSize m_minimum_block_size;
        VkAllocationCallbacks const* m_vulkan_allocator;
        std::pmr::polymorphic_allocator<bool> m_bool_allocator;
    };
}
