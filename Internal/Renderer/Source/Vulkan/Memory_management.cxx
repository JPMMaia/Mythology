module;

#include <vulkan/vulkan.h>

#include <memory_resource>
#include <optional>
#include <unordered_map>
#include <vector>

export module maia.renderer.vulkan.memory_management;

import maia.renderer.vulkan.device_memory;
namespace Maia::Renderer::Vulkan
{
    export struct Device_memory_range
    {
        VkDeviceMemory device_memory;
        VkDeviceSize offset;
        VkDeviceSize size;
    };
    
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

        std::pmr::unordered_multimap<Memory_type_index, Chunk, Memory_type_index_hash> m_chunks;
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

        std::pmr::unordered_multimap<Memory_type_index, Chunk, Memory_type_index_hash> m_chunks;
        VkDevice m_device;
        VkDeviceSize m_maximum_block_size;
        VkDeviceSize m_minimum_block_size;
        VkAllocationCallbacks const* m_vulkan_allocator;
        std::pmr::polymorphic_allocator<bool> m_bool_allocator;
    };


    export struct Buffer_pool_node
    {
        Memory_tree const* memory_tree;
        Tree_node node;

        VkDeviceSize offset() const noexcept;
        VkDeviceSize size() const noexcept;
    };

    export struct Device_memory_and_properties
    {
        VkDeviceMemory device_memory;
        VkMemoryPropertyFlags properties;
    };
    
    export class Buffer_pool_memory_resource
    {
    public:

        Buffer_pool_memory_resource(
            VkPhysicalDeviceMemoryProperties const& physical_device_memory_properties,
            VkDevice device,
            VkBufferCreateInfo const& buffer_create_info,
            VkMemoryPropertyFlags memory_properties,
            VkDeviceSize minimum_block_size,
            VkAllocationCallbacks const* vulkan_allocator = nullptr,
            std::pmr::polymorphic_allocator<bool> const& bool_allocator = {}
        ) noexcept;
        Buffer_pool_memory_resource(Buffer_pool_memory_resource const&) noexcept = delete;
        Buffer_pool_memory_resource(Buffer_pool_memory_resource&& other) noexcept;
        ~Buffer_pool_memory_resource() noexcept;

        Buffer_pool_memory_resource& operator=(Buffer_pool_memory_resource const&) noexcept = delete;
        Buffer_pool_memory_resource& operator=(Buffer_pool_memory_resource&& other) noexcept;

        std::optional<Buffer_pool_node> allocate(
            VkDeviceSize bytes_to_allocate,
            VkDeviceSize alignment,
            VkBufferUsageFlags buffer_usage_flags
        ) noexcept;

        void deallocate(
            Buffer_pool_node memory_resource
        ) noexcept;

        
        VkBuffer buffer() noexcept;

        VkDeviceMemory device_memory() noexcept;

        VkMemoryPropertyFlags memory_properties() noexcept;

    private:

        VkDevice m_device;
        VkBuffer m_buffer;
        VkBufferUsageFlags m_buffer_usage_flags;
        Device_memory_and_properties m_device_memory_and_properties;
        Memory_tree m_memory_tree;
        VkAllocationCallbacks const* m_vulkan_allocator;
    };
}
