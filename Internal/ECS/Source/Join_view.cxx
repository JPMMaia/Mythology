export module join_view;

import <ranges>;
import <type_traits>;

namespace details
{
    template<std::ranges::input_range R> requires std::ranges::view<R>
    class Join_iterator
    {
    private:

        std::ranges::iterator_t<R> m_current_base;
        std::ranges::iterator_t<R> m_base_end;

        using Underlying_iterator_t = std::ranges::iterator_t<typename std::ranges::iterator_t<R>::value_type>;

        Underlying_iterator_t m_current_underlying;
        Underlying_iterator_t m_underlying_end;

    public:

        using difference_type = std::ptrdiff_t;
        using value_type = typename Underlying_iterator_t::value_type;
        using pointer = typename Underlying_iterator_t::pointer;
        using reference = typename Underlying_iterator_t::reference;
        using iterator_category = std::input_iterator_tag;

        Join_iterator() noexcept = default;

        Join_iterator(std::ranges::iterator_t<R> base, std::ranges::iterator_t<R> base_end) noexcept :
            m_current_base{base},
            m_base_end{base_end},
            m_current_underlying{base != base_end ? base->begin() : Underlying_iterator_t{}},
            m_underlying_end{base != base_end ? base->end() : Underlying_iterator_t{}}
        {
        }

        Join_iterator(std::ranges::iterator_t<R> base, std::ranges::iterator_t<R> base_end, Underlying_iterator_t current, Underlying_iterator_t end) noexcept :
            m_current_base{base},
            m_base_end{base_end},
            m_current_underlying{current},
            m_underlying_end{end}
        {
        }

        reference operator*() const noexcept
        {
            return *m_current_underlying;
        }

        pointer operator->() const noexcept
        {
            return m_current_underlying;
        }

        bool operator==(Join_iterator const rhs) const noexcept
        {
            return m_current_underlying == rhs.m_current_underlying;
        }

        Join_iterator& operator++() noexcept
        {
            ++m_current_underlying;

            if (m_current_underlying != m_underlying_end) [[likely]]
            {
                return *this;
            }

            ++m_current_base;

            if (m_current_base != m_base_end) [[likely]]
            {
                m_current_underlying = std::begin(*m_current_base);
                m_underlying_end = std::end(*m_current_base);
            }
            else
            {
                m_current_underlying = Underlying_iterator_t{};
                m_underlying_end = Underlying_iterator_t{};
            }

            return *this;
        }

        Join_iterator operator++(int) noexcept
        {
            Join_iterator const temp
            {
                m_current_base,
                m_base_end,
                m_current_underlying,
                m_underlying_end
            };

            ++(*this);
            
            return temp;
        }
    };

    export template<std::ranges::input_range R> requires std::ranges::view<R>
    class Join_view : public std::ranges::view_interface<Join_view<R>>
    {
    public:
        
        constexpr Join_view() = default;
        
        constexpr Join_view(R base) : 
            m_base{base},
            m_begin{std::begin(m_base), std::end(m_base)},
            m_end{std::end(m_base), std::end(m_base)}
        {
        }
        
        constexpr R base() const &
        {
            return m_base;
        }
        constexpr R base() && 
        {
            return std::move(m_base);
        }
        
        constexpr auto begin() const
        {
            return m_begin;
        }
        constexpr auto end() const
        {
            return m_end;
        }

    private:

        R m_base{};
        Join_iterator<R> m_begin{};
        Join_iterator<R> m_end{};

    };
    
    template<class R>
    Join_view(R&& base)
        -> Join_view<std::ranges::views::all_t<R>>;

    struct Join_adaptor_closure
    {
        template <std::ranges::viewable_range R>
        constexpr auto operator()(R && r) const
        {
            return Join_view(std::forward<R>(r));
        }
    } ;

    export template <std::ranges::viewable_range R>
    constexpr auto operator | (R&& r, Join_adaptor_closure const & a)
    {
        return a(std::forward<R>(r)) ;
    }
}

namespace views
{
    export details::Join_adaptor_closure Join;
}