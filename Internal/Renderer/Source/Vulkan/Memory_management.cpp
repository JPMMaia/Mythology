module maia.renderer.vulkan.memory_management;

import maia.renderer.vulkan.allocation_callbacks;
import maia.renderer.vulkan.device;
import maia.renderer.vulkan.device_memory;

import <vulkan/vulkan.h>;

import <cassert>;
import <cmath>;
import <optioanl>;
import <memory_resource>;
import <unordered_map>;
import <utility>;

namespace Maia::Renderer::Vulkan
{
    Device_memory_range allocate_device_memory(
        VkDevice const device,
        Memory_type_index const memory_type_index,
        VkDeviceSize const allocation_size,
        VkAllocationCallbacks const* const vulkan_allocator
    ) noexcept
    {   
        VkDeviceMemory const device_memory = 
            allocate_memory({device}, allocation_size, memory_type_index, vulkan_allocator ? Allocation_callbacks{*vulkan_allocator} : Allocation_callbacks{VK_NULL_HANDLE}).value;
        
        return {device_memory, 0, allocation_size};
    }

    void free_device_memory(
        VkDevice const device,
        Device_memory_range const device_memory_range,
        VkAllocationCallbacks const* const vulkan_allocator
    ) noexcept
    {
        free_memory({device}, {device_memory_range.device_memory}, vulkan_allocator ? Allocation_callbacks{*vulkan_allocator} : Allocation_callbacks{VK_NULL_HANDLE});
    }

    Monotonic_device_memory_resource::Monotonic_device_memory_resource(
        VkDevice const device,
        VkDeviceSize const blocks_size,
        VkAllocationCallbacks const* const vulkan_allocator,
        std::pmr::polymorphic_allocator<std::pair<Memory_type_index const, Chunk>> const& allocator
    ) noexcept :
        m_chunks{allocator},
        m_device{device},
        m_blocks_size{blocks_size},
        m_vulkan_allocator{vulkan_allocator}
    {
    }
    Monotonic_device_memory_resource::Monotonic_device_memory_resource(Monotonic_device_memory_resource&& other) noexcept :
        m_chunks{std::exchange(other.m_chunks, {})},
        m_device{std::exchange(other.m_device, {VK_NULL_HANDLE})},
        m_blocks_size{std::exchange(other.m_blocks_size, 0)},
        m_vulkan_allocator{std::exchange(other.m_vulkan_allocator, nullptr)}
    {
    }
    Monotonic_device_memory_resource::~Monotonic_device_memory_resource() noexcept
    {
        for (std::pair<Memory_type_index const, Chunk> const& pair : m_chunks)
        {
            Chunk const& chunk = pair.second;
            free_device_memory(m_device, chunk.device_memory_range, m_vulkan_allocator);
        }
    }

    Monotonic_device_memory_resource& Monotonic_device_memory_resource::operator=(Monotonic_device_memory_resource&& other) noexcept
    {
        std::swap(m_chunks, other.m_chunks);
        std::swap(m_device, other.m_device);
        std::swap(m_blocks_size, other.m_blocks_size);
        std::swap(m_vulkan_allocator, other.m_vulkan_allocator);

        return *this;
    }

    Device_memory_range Monotonic_device_memory_resource::allocate(
        Memory_type_index const memory_type_index,
        VkDeviceSize const bytes_to_allocate,
        VkDeviceSize const alignment
    ) noexcept
    {
        auto const range = m_chunks.equal_range(memory_type_index);

        auto const is_chunk_available = [bytes_to_allocate](std::pair<Memory_type_index, Chunk> const& pair) -> bool
        {
            Chunk const& chunk = pair.second;
            return (chunk.allocated_bytes + bytes_to_allocate) <= chunk.device_memory_range.size;
        };

        auto const chunk_available_iterator = std::find_if(range.first, range.second, is_chunk_available);

        if (chunk_available_iterator != range.second)
        {
            Chunk& chunk = chunk_available_iterator->second;

            VkDeviceSize const offset_to_new_range = 
                chunk.device_memory_range.offset + chunk.allocated_bytes;

            chunk.allocated_bytes += bytes_to_allocate;
            assert(chunk.allocated_bytes <= chunk.device_memory_range.size);

            return {chunk.device_memory_range.device_memory, offset_to_new_range, bytes_to_allocate};
        }
        else
        {
            Device_memory_range const new_device_memory_range =
                allocate_device_memory(m_device, memory_type_index, m_blocks_size, m_vulkan_allocator);

            {
                Chunk const new_chunk{new_device_memory_range, bytes_to_allocate};
                m_chunks.insert({memory_type_index, new_chunk});
            }

            return {new_device_memory_range.device_memory, new_device_memory_range.offset, bytes_to_allocate};
        }
    }


    namespace
    {
        std::uint32_t constexpr block_count(Tree_level const level) noexcept
        {
            return static_cast<Memory_size>(std::pow(2, level.value));
        }

        Tree_node_index constexpr get_node_index(
            Tree_node const node
        ) noexcept
        {
            return {static_cast<std::uint32_t>(std::pow(2, node.level.value) - 1 + node.level_index.value)}; 
        }

        Tree_node constexpr get_left_child(Tree_node const node) noexcept
        {
            return
            {
                .level = {static_cast<std::uint8_t>(node.level.value + 1)},
                .level_index = {2 * node.level_index.value}
            };
        }

        Tree_node constexpr get_right_child(Tree_node const node) noexcept
        {
            return
            {
                .level = {static_cast<std::uint8_t>(node.level.value + 1)},
                .level_index = {2 * node.level_index.value + 1}
            };
        }

        Tree_node constexpr get_parent(Tree_node const node) noexcept
        {
            assert(node.level.value != 0);

            return
            {
                .level = {static_cast<std::uint8_t>(node.level.value - 1)},
                .level_index = {node.level_index.value / 2}
            };
        }

        Tree_node constexpr get_sibling(Tree_node const node) noexcept
        {
            assert(node.level.value != 0);

            bool const is_left_child = !(node.level_index.value & 1);

            return is_left_child ?
                get_right_child(get_parent(node)) :
                get_left_child(get_parent(node));
        }

        Memory_size constexpr block_size(Tree_level const level, Memory_size const maximum_block_size) noexcept
        {
            assert((maximum_block_size % 2) == 0);

            return maximum_block_size / block_count(level);
        }

        Memory_size constexpr begin_offset(Tree_node const node, Memory_size const maximum_block_size) noexcept
        {
            return node.level_index.value * block_size(node.level, maximum_block_size);
        }

        Memory_size constexpr end_offset(Tree_node const node, Memory_size const maximum_block_size) noexcept
        {
            return (node.level_index.value + 1) * block_size(node.level, maximum_block_size);
        }

        Tree_level get_level_with_blocks(
            Memory_size const required_size,
            Memory_size const minimum_block_size,
            Memory_size const maximum_block_size) noexcept
        {
            assert((maximum_block_size % 2) == 0);

            return {static_cast<std::uint8_t>(std::log2(maximum_block_size / std::max(required_size, minimum_block_size)))};
        }

        std::optional<Tree_node> search_depth_first(Memory_tree const& tree, Tree_node const base) noexcept
        {
            Tree_node_index const base_index = get_node_index(base);

            if (!tree.allocated[base_index.value] && !tree.internal[base_index.value])
            {
                return base;
            }

            else if (!tree.allocated[base_index.value] && tree.internal[base_index.value])
            {
                {
                    Tree_node const left_child = get_left_child(base);
                    
                    std::optional<Tree_node> const node_on_left_side = search_depth_first(tree, left_child);
                    
                    if (node_on_left_side)
                    {
                        return node_on_left_side;
                    }
                }

                {
                    Tree_node const right_child = get_right_child(base);
                    
                    std::optional<Tree_node> const node_on_right_side = search_depth_first(tree, right_child);
                    
                    if (node_on_right_side)
                    {
                        return node_on_right_side;
                    }
                }
            }
            
            return {};
        }

        bool constexpr is_free_external_node(Memory_tree const& tree, Tree_node const node) noexcept
        {
            Tree_node_index const node_index = get_node_index(node);
            
            return !tree.internal[node_index.value] && !tree.allocated[node_index.value];
        }

        bool constexpr is_allocated_external_node(Memory_tree const& tree, Tree_node const node) noexcept
        {
            Tree_node_index const node_index = get_node_index(node);
            
            return !tree.internal[node_index.value] && tree.allocated[node_index.value];
        }

        bool constexpr is_free_internal_node(Memory_tree const& tree, Tree_node const node) noexcept
        {
            Tree_node_index const node_index = get_node_index(node);
            
            return tree.internal[node_index.value] && !tree.allocated[node_index.value];
        }

        bool constexpr is_allocated_node(Memory_tree const& tree, Tree_node const node) noexcept
        {
            Tree_node_index const node_index = get_node_index(node);
            
            return tree.allocated[node_index.value];
        }

        bool constexpr is_free_node(Memory_tree const& tree, Tree_node const node) noexcept
        {
            Tree_node_index const node_index = get_node_index(node);
            
            return !tree.allocated[node_index.value];
        }

        void split_subtree(Memory_tree& tree, Tree_node const parent) noexcept
        {
            assert(is_free_external_node(tree, parent));

            Tree_node_index const parent_index = get_node_index(parent);

            tree.internal[parent_index.value] = true;

            assert(is_free_external_node(tree, get_left_child(parent)));
            assert(is_free_external_node(tree, get_right_child(parent)));
            assert(is_free_internal_node(tree, parent));
        }

        void join_subtree(Memory_tree& tree, Tree_node const parent) noexcept
        {
            assert(is_free_internal_node(tree, parent));
            assert(is_free_external_node(tree, get_right_child(parent)));
            assert(is_free_external_node(tree, get_left_child(parent)));

            Tree_node_index const parent_index = get_node_index(parent);

            tree.internal[parent_index.value] = false;

            assert(is_free_external_node(tree, parent));
        }

        Tree_node split_subtrees_until_level(Memory_tree& tree, Tree_node const root, Tree_level const level) noexcept
        {
            assert(root.level.value < level.value);
            assert(get_node_index({level, 0}).value < !tree.internal.size());
            assert(is_free_external_node(tree, root));

            Tree_node current_node = root;

            while (current_node.level.value != level.value)
            {
                assert(is_free_external_node(tree, current_node));

                split_subtree(tree, current_node);
                
                current_node = get_left_child(current_node);
                assert(is_free_external_node(tree, current_node));
            }

            assert(is_free_external_node(tree, current_node));
            assert(current_node.level.value == level.value);
            return current_node;
        }

        void join_free_subtrees(Memory_tree& tree, Tree_node const external_node) noexcept
        {
            assert(is_free_external_node(tree, external_node));

            Tree_node current_node = external_node;

            while (is_free_external_node(tree, get_sibling(current_node)))
            {
                assert(is_free_external_node(tree, current_node));
                
                Tree_node const parent = get_parent(current_node);
                join_subtree(tree, parent);
                
                current_node = parent;
                assert(is_free_external_node(tree, current_node));
            }
        }

        /*std::optional<Tree_node> allocate_node(Memory_tree& tree, Memory_size const required_size) noexcept
        {
            Tree_node const root{0, 0};

            std::optional<Tree_node> const free_node = search_depth_first(tree, root);

            if (free_node)
            {
                Tree_level const requested_level = 
                    get_level_with_blocks(required_size, tree.minimum_block_size, tree.maximum_block_size);

                Tree_node const free_node_of_requested_level = split_subtrees_until_level(tree, *free_node, requested_level);

                allocate_node(tree, free_node_of_requested_level);
                
                return free_node_of_requested_level;
            }
            {
                return {};
            }
        }*/
    }

    std::optional<Tree_node> find_free_node(
        Memory_tree const& tree,
        Tree_node const base,
        Memory_size const required_size
    ) noexcept
    {
        // TODO find free node such that lower_level_block_size < required_size <= level_block_size

        std::optional<Tree_node> const free_node = search_depth_first(tree, base);

        if (free_node)
        {
            Tree_level const requested_level = 
                get_level_with_blocks(required_size, tree.minimum_block_size, tree.maximum_block_size);


            // TODO do not split. instead just find index of node (as if we were to split)
            Tree_node const free_node_of_requested_level = split_subtrees_until_level(tree, *free_node, requested_level);

            return free_node_of_requested_level;
        }
        else
        {
            return {};
        }
    }

    void allocate_node(
        Memory_tree& tree,
        Tree_node const node_to_allocate
    ) noexcept
    {
        assert(is_free_external_node(tree, node_to_allocate));

        // TODO split subtrees

        {
            Tree_node_index const index = get_node_index(node_to_allocate);
            tree.allocated[index.value] = true;
        }

        {
            Tree_node current_node = node_to_allocate;

            while (is_allocated_node(tree, get_sibling(current_node)))
            {
                assert(is_allocated_node(tree, current_node));

                Tree_node const parent = get_parent(current_node);
                Tree_node_index const parent_index = get_node_index(parent);
                tree.allocated[parent_index.value] = true;

                current_node = parent;
                assert(is_allocated_node(tree, current_node));
            }
        }

        assert(is_allocated_external_node(tree, node_to_allocate));
    }

    void free_node(
        Memory_tree& tree,
        Tree_node const node_to_free
    ) noexcept
    {
        assert(is_allocated_external_node(tree, node_to_free));

        {
            Tree_node_index const index = get_node_index(node_to_free);
            tree.allocated[index.value] = false;
        }

        {
            Tree_node current_node = node_to_free;

            while (is_allocated_node(tree, get_parent(current_node)))
            {
                assert(is_free_node(tree, current_node));

                Tree_node const parent = get_parent(current_node);
                Tree_node_index const parent_index = get_node_index(parent);
                tree.allocated[parent_index.value] = false;

                current_node = parent;
                assert(is_free_node(tree, current_node));
            }
        }

        join_free_subtrees(tree, node_to_free);

        assert(is_free_external_node(tree, node_to_free));
    }
}
