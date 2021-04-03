module;

#include <cstddef>

export module maia.test.debug_object;

namespace Maia::Test
{
    export class Debug_object
    {
    public:

        Debug_object(
            std::size_t* const constructor_counter,
            std::size_t* const destructor_counter
        ) noexcept :
            m_constructor_counter{constructor_counter},
            m_destructor_counter{destructor_counter}
        {
            ++(*m_constructor_counter);
        }

        Debug_object(Debug_object const& other) noexcept :
            m_constructor_counter{other.m_constructor_counter},
            m_destructor_counter{other.m_destructor_counter}
        {
            ++(*m_constructor_counter);
        }

        Debug_object(Debug_object&& other) noexcept :
            m_constructor_counter{other.m_constructor_counter},
            m_destructor_counter{other.m_destructor_counter}
        {
            ++(*m_constructor_counter);
        }

        ~Debug_object() noexcept
        {
            ++(*m_destructor_counter);
        }

    private:

        std::size_t* m_constructor_counter;
        std::size_t* m_destructor_counter;

    };
}
