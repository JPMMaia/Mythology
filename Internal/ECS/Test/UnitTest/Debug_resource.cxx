module;

#include <cstddef>
#include <memory_resource>

export module maia.test.debug_resource;

namespace Maia::Test
{
    export class Debug_resource final : public std::pmr::memory_resource
    {
    public:

        Debug_resource(
            std::size_t* const allocated_bytes_counter,
            std::size_t* const deallocated_bytes_counter,
            std::pmr::memory_resource* const upstream = std::pmr::get_default_resource()
        ) noexcept :
            Debug_resource(allocated_bytes_counter, deallocated_bytes_counter, nullptr, upstream)
        {
        }

        Debug_resource(
            std::size_t* const allocated_bytes_counter,
            std::size_t* const deallocated_bytes_counter,
            std::size_t* const remaining_memory,
            std::pmr::memory_resource* const upstream = std::pmr::get_default_resource()
        ) noexcept :
            m_allocated_bytes_counter{allocated_bytes_counter},
            m_deallocated_bytes_counter{deallocated_bytes_counter},
            m_remaining_memory{remaining_memory},
            m_upstream{upstream}
        {
        }

        void* do_allocate(std::size_t const bytes, std::size_t const alignment) final
        {
            if (m_remaining_memory != nullptr)
            {
                if (bytes <= *m_remaining_memory)
                {
                    *m_remaining_memory -= bytes;
                }
                else
                {
                    throw std::bad_alloc{};
                }
            }

            *m_allocated_bytes_counter += bytes;

            return m_upstream->allocate(bytes, alignment);
        }

        void do_deallocate(void* const ptr, std::size_t const bytes, std::size_t const alignment) final
        {
            if (m_remaining_memory != nullptr)
            {
                *m_remaining_memory -= bytes;
            }

            *m_deallocated_bytes_counter += bytes;

            m_upstream->deallocate(ptr, bytes, alignment);
        }

        bool do_is_equal(std::pmr::memory_resource const& other) const noexcept final
        {
            return m_upstream->is_equal(other);
        }

    private:

        std::size_t* m_allocated_bytes_counter;
        std::size_t* m_deallocated_bytes_counter;
        std::size_t* m_remaining_memory;
        std::pmr::memory_resource* m_upstream;

    };
}
