module maia.renderer.vulkan.memory_management;

import maia.renderer.vulkan.buffer;
import maia.renderer.vulkan.device_memory;

import <vulkan/vulkan.h>;

import <cassert>;
import <cmath>;
import <optional>;
import <memory_resource>;
import <unordered_map>;
import <utility>;

namespace Maia::Renderer::Vulkan
{
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
            free_memory(m_device, chunk.device_memory_range.device_memory, m_vulkan_allocator);
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

    namespace
    {
        VkDeviceSize align(VkDeviceSize const offset, VkDeviceSize const alignment) noexcept
        {
            assert((alignment == 1) || (alignment % 2 == 0));

            return (offset + alignment -1) & ~(alignment-1);;
        }
    }

    Device_memory_range Monotonic_device_memory_resource::allocate(
        Memory_type_index const memory_type_index,
        VkDeviceSize bytes_to_allocate,
        VkDeviceSize const alignment
    ) noexcept
    {
        bytes_to_allocate = align(bytes_to_allocate, alignment);
        assert((bytes_to_allocate % alignment) == 0);

        auto const range = m_chunks.equal_range(memory_type_index);

        auto const is_chunk_available = [bytes_to_allocate, alignment](std::pair<Memory_type_index, Chunk> const& pair) -> bool
        {
            Chunk const& chunk = pair.second;
            
            VkDeviceSize const aligned_offset = align(chunk.allocated_bytes, alignment);
            return (aligned_offset + bytes_to_allocate) <= chunk.device_memory_range.size;
        };

        auto const chunk_available_iterator = std::find_if(range.first, range.second, is_chunk_available);

        if (chunk_available_iterator != range.second)
        {
            Chunk& chunk = chunk_available_iterator->second;

            VkDeviceSize const offset_to_new_range = 
                align(chunk.device_memory_range.offset + chunk.allocated_bytes, alignment);

            chunk.allocated_bytes = (offset_to_new_range + bytes_to_allocate) - chunk.device_memory_range.offset;
            assert(chunk.allocated_bytes <= chunk.device_memory_range.size);

            assert((bytes_to_allocate % alignment) == 0);
            assert((offset_to_new_range % alignment) == 0);
            return {chunk.device_memory_range.device_memory, offset_to_new_range, bytes_to_allocate};
        }
        else
        {
            VkDeviceMemory const new_device_memory =
                allocate_memory(m_device, m_blocks_size, memory_type_index, m_vulkan_allocator);

            Device_memory_range const new_device_memory_range{new_device_memory, 0, m_blocks_size};

            {
                Chunk const new_chunk{new_device_memory_range, bytes_to_allocate};
                m_chunks.insert({memory_type_index, new_chunk});
            }

            assert((bytes_to_allocate % alignment) == 0);
            assert((new_device_memory_range.offset % alignment) == 0);
            return {new_device_memory_range.device_memory, new_device_memory_range.offset, bytes_to_allocate};
        }
    }


    namespace
    {
        std::uint32_t constexpr block_count(Memory_size const minimum_block_size, Memory_size const maximum_block_size) noexcept
        {
            assert(minimum_block_size <= maximum_block_size);
            assert(minimum_block_size != 0);
            assert((minimum_block_size % 2) == 0);
            assert((maximum_block_size % 2) == 0);

            return 2*(maximum_block_size / minimum_block_size) - 1;
        }

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
            assert(get_node_index({level, 0}).value < tree.internal.size());
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
            assert(external_node.level.value != 0);

            Tree_node current_node = external_node;

            while (current_node.level.value != 0 && is_free_external_node(tree, get_sibling(current_node)))
            {
                assert(is_free_external_node(tree, current_node));
                
                Tree_node const parent = get_parent(current_node);
                join_subtree(tree, parent);
                
                current_node = parent;
                assert(is_free_external_node(tree, current_node));
            }
        }

        std::optional<Tree_node> find_free_node(
            Memory_tree const& tree,
            Tree_node const base,
            Memory_size const required_size
        ) noexcept
        {
            std::optional<Tree_node> const free_node = search_depth_first(tree, base);

            if (free_node)
            {
                Tree_level const requested_level = 
                    get_level_with_blocks(required_size, tree.minimum_block_size, tree.maximum_block_size);

                Tree_node const free_node_of_requested_level = {
                    .level = requested_level,
                    .level_index = {static_cast<std::uint32_t>(std::pow(2, requested_level.value - free_node->level.value) * free_node->level_index.value)}
                };

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
    }

    Memory_tree create_memory_tree(
        Memory_size minimum_block_size,
        Memory_size maximum_block_size,
        std::pmr::polymorphic_allocator<bool> const& allocator
    ) noexcept
    {
        assert(minimum_block_size <= minimum_block_size);
        assert(minimum_block_size != 0);
        assert((minimum_block_size % 2) == 0);
        assert((maximum_block_size % 2) == 0);

        std::uint32_t const num_blocks = block_count(minimum_block_size, maximum_block_size);

        return
        {
            .allocated{num_blocks, 0, allocator},
            .internal{num_blocks, 0, allocator},
            .minimum_block_size = minimum_block_size,
            .maximum_block_size = maximum_block_size
        };
    }

    std::optional<Tree_node> allocate_node(
        Memory_tree& tree,
        Memory_size const required_size
    ) noexcept
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


    Pool_device_memory_resource::Pool_device_memory_resource(
        VkDevice const device,
        VkDeviceSize const maximum_block_size,
        VkDeviceSize const minimum_block_size,
        VkAllocationCallbacks const* const vulkan_allocator,
        std::pmr::polymorphic_allocator<std::pair<Memory_type_index const, Chunk>> const& chunk_allocator,
        std::pmr::polymorphic_allocator<bool> bool_allocator
    ) noexcept :
        m_chunks{chunk_allocator},
        m_device{device},
        m_maximum_block_size{maximum_block_size},
        m_minimum_block_size{minimum_block_size},
        m_vulkan_allocator{vulkan_allocator},
        m_bool_allocator{std::move(bool_allocator)}
    {
    }
    Pool_device_memory_resource::Pool_device_memory_resource(Pool_device_memory_resource&& other) noexcept :
        m_chunks{std::exchange(other.m_chunks, {})},
        m_device{other.m_device},
        m_maximum_block_size{other.m_maximum_block_size},
        m_minimum_block_size{other.m_minimum_block_size},
        m_vulkan_allocator{std::exchange(other.m_vulkan_allocator, {})}
    {
    }
    Pool_device_memory_resource::~Pool_device_memory_resource() noexcept
    {
        for (std::pair<Memory_type_index const, Chunk> const& pair : m_chunks)
        {
            Chunk const& chunk = pair.second;
            free_memory(m_device, chunk.device_memory_range.device_memory, m_vulkan_allocator);
        }
    }

    Pool_device_memory_resource& Pool_device_memory_resource::operator=(Pool_device_memory_resource&& other) noexcept
    {
        std::swap(m_chunks, other.m_chunks);
        std::swap(m_device, other.m_device);
        std::swap(m_maximum_block_size, other.m_maximum_block_size);
        std::swap(m_minimum_block_size, other.m_minimum_block_size);
        std::swap(m_vulkan_allocator, other.m_vulkan_allocator);

        return *this;
    }

    namespace
    {
        Device_memory_range to_device_memory_range(
            VkDeviceMemory const device_memory,
            Tree_node const node,
            Memory_size const maximum_block_size
        ) noexcept
        {
            return
            {
                .device_memory = device_memory,
                .offset = begin_offset(node, maximum_block_size),
                .size = block_size(node.level, maximum_block_size)
            };
        }
    }

    Node_device_memory_resource Pool_device_memory_resource::allocate(
        Memory_type_index const memory_type_index,
        VkDeviceSize const bytes_to_allocate,
        VkDeviceSize const alignment
    ) noexcept
    {
        assert(bytes_to_allocate <= m_maximum_block_size);
        assert((alignment % m_minimum_block_size) == 0);

        auto const range = m_chunks.equal_range(memory_type_index);

        for (auto iterator = range.first; iterator != range.second; ++iterator)
        {
            Chunk& chunk = iterator->second;

            std::optional<Tree_node> const node = allocate_node(chunk.memory_tree, bytes_to_allocate);

            if (node)
            {
                return
                {
                    .device_memory_range = to_device_memory_range(chunk.device_memory_range.device_memory, *node, m_maximum_block_size),
                    .memory_tree = chunk.memory_tree,
                    .node = *node
                };
            }
        }

        VkDeviceMemory const device_memory = 
            allocate_memory(m_device, m_maximum_block_size, memory_type_index, m_vulkan_allocator);

        auto const chunkIterator = m_chunks.emplace(memory_type_index, Chunk{{device_memory, 0, m_maximum_block_size}, create_memory_tree(m_minimum_block_size, m_maximum_block_size, m_bool_allocator)});
        Memory_tree& tree = chunkIterator->second.memory_tree;
        
        std::optional<Tree_node> const node = allocate_node(tree, bytes_to_allocate);
        assert(node.has_value());

        return
        {
            .device_memory_range = to_device_memory_range(chunkIterator->second.device_memory_range.device_memory, *node, m_maximum_block_size),
            .memory_tree = tree,
            .node = *node
        };
    }

    void Pool_device_memory_resource::deallocate(
        Node_device_memory_resource const memory_resource
    ) noexcept
    {
        assert(std::find_if(m_chunks.begin(), m_chunks.end(),
            [&memory_resource](std::pair<Memory_type_index, Chunk> const& pair) -> bool { return &pair.second.memory_tree == &memory_resource.memory_tree; }) != m_chunks.end());

        free_node(memory_resource.memory_tree, memory_resource.node);
    }


    VkDeviceSize Buffer_pool_node::offset() const noexcept
    {
        return begin_offset(this->node, this->memory_tree->maximum_block_size);
    }
    VkDeviceSize Buffer_pool_node::size() const noexcept
    {
        return block_size(this->node.level, this->memory_tree->maximum_block_size);
    }


    namespace
    {
        Device_memory_and_properties allocate_buffer_device_memory(
            VkPhysicalDeviceMemoryProperties const& physical_device_memory_properties,
            VkDevice const device,
            VkBuffer const buffer,
            VkMemoryPropertyFlags const memory_properties,
            VkAllocationCallbacks const* const vulkan_allocator
        ) noexcept
        {
            VkMemoryRequirements const memory_requirements = 
                get_memory_requirements(device, buffer).value;

            Memory_type_bits const memory_type_bits = get_memory_type_bits({memory_requirements});

            std::optional<Memory_type_index_and_properties> const memory_type_index_and_properties = 
                find_memory_type({physical_device_memory_properties}, memory_type_bits, memory_properties);

            assert(memory_type_index_and_properties.has_value());

            VkDeviceMemory const device_memory = allocate_memory(
                device,
                memory_requirements.size,
                memory_type_index_and_properties->type_index,
                vulkan_allocator
            );

            return {device_memory, memory_type_index_and_properties->properties};
        }
    }

    Buffer_pool_memory_resource::Buffer_pool_memory_resource(
        VkPhysicalDeviceMemoryProperties const& physical_device_memory_properties,
        VkDevice const device,
        VkBufferCreateInfo const& buffer_create_info,
        VkMemoryPropertyFlags const memory_properties,
        VkDeviceSize const minimum_block_size,
        VkAllocationCallbacks const* const vulkan_allocator,
        std::pmr::polymorphic_allocator<bool> const& bool_allocator
    ) noexcept :
        m_device{device},
        m_buffer{create_buffer(device, buffer_create_info, vulkan_allocator)},
        m_buffer_usage_flags{buffer_create_info.usage},
        m_device_memory_and_properties{allocate_buffer_device_memory(physical_device_memory_properties, device, m_buffer, memory_properties, vulkan_allocator)},
        m_memory_tree{create_memory_tree(minimum_block_size, buffer_create_info.size, bool_allocator)},
        m_vulkan_allocator{vulkan_allocator}
    {
        bind_memory(m_device, m_buffer, m_device_memory_and_properties.device_memory, 0);
    }
    Buffer_pool_memory_resource::Buffer_pool_memory_resource(Buffer_pool_memory_resource&& other) noexcept :
        m_device{std::exchange(other.m_device, {})},
        m_buffer{std::exchange(other.m_buffer, {})},
        m_buffer_usage_flags{std::exchange(other.m_buffer_usage_flags, {})},
        m_device_memory_and_properties{std::exchange(other.m_device_memory_and_properties, {})},
        m_memory_tree{std::exchange(other.m_memory_tree, {})},
        m_vulkan_allocator{std::exchange(other.m_vulkan_allocator, {})}
    {
    }
    Buffer_pool_memory_resource::~Buffer_pool_memory_resource() noexcept
    {
        if (m_device_memory_and_properties.device_memory != VK_NULL_HANDLE)
        {
            free_memory(m_device, m_device_memory_and_properties.device_memory, m_vulkan_allocator);
        }

        if (m_buffer != VK_NULL_HANDLE)
        {
            destroy_buffer(m_device, m_buffer, m_vulkan_allocator);
        }
    }

    Buffer_pool_memory_resource& Buffer_pool_memory_resource::operator=(Buffer_pool_memory_resource&& other) noexcept
    {
        std::swap(m_device, other.m_device);
        std::swap(m_buffer, other.m_buffer);
        std::swap(m_buffer_usage_flags, other.m_buffer_usage_flags);
        std::swap(m_device_memory_and_properties, other.m_device_memory_and_properties);
        std::swap(m_memory_tree, other.m_memory_tree);
        std::swap(m_vulkan_allocator, other.m_vulkan_allocator);

        return *this;
    }

    std::optional<Buffer_pool_node> Buffer_pool_memory_resource::allocate(
        VkDeviceSize const bytes_to_allocate,
        VkDeviceSize const alignment,
        VkBufferUsageFlags const buffer_usage_flags
    ) noexcept
    {
        assert(bytes_to_allocate <= m_memory_tree.maximum_block_size);
        assert((alignment % m_memory_tree.minimum_block_size) == 0);
        assert((buffer_usage_flags | m_buffer_usage_flags) == m_buffer_usage_flags);

        std::optional<Tree_node> const node = allocate_node(m_memory_tree, bytes_to_allocate);

        if (node)
        {
            return
            {
                {
                    .memory_tree = &m_memory_tree,
                    .node = *node
                }
            };
        }
        else
        {
            return {};
        }
    }

    void Buffer_pool_memory_resource::deallocate(
        Buffer_pool_node const memory_resource
    ) noexcept
    {
        assert(memory_resource.memory_tree != nullptr);
        assert(&m_memory_tree == memory_resource.memory_tree);

        free_node(m_memory_tree, memory_resource.node);
    }

    VkBuffer Buffer_pool_memory_resource::buffer() noexcept
    {
        return m_buffer;
    }

    VkDeviceMemory Buffer_pool_memory_resource::device_memory() noexcept
    {
        return m_device_memory_and_properties.device_memory;
    }

    VkMemoryPropertyFlags Buffer_pool_memory_resource::memory_properties() noexcept
    {
        return m_device_memory_and_properties.properties;
    }
}
