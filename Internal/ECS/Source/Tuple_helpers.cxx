module;

#include <tuple>
#include <vector>

export module maia.ecs.tuple_helpers;

namespace Maia::ECS
{
    export template <typename T, typename Tuple>
    struct Has_type;

    export template <typename T>
    struct Has_type<T, std::tuple<>> : std::false_type {};

    export template <typename T, typename U, typename... Ts>
    struct Has_type<T, std::tuple<U, Ts...>> : Has_type<T, std::tuple<Ts...>> {};

    export template <typename T, typename... Ts>
    struct Has_type<T, std::tuple<T, Ts...>> : std::true_type {};

    
    export template <class T, class Tuple>
    struct Tuple_element_index;

    export template <class T, class... Types>
    struct Tuple_element_index<T, std::tuple<T, Types...>>
    {
        static constexpr std::size_t value = 0;
    };

    export template <class T, class U, class... Types>
    struct Tuple_element_index<T, std::tuple<U, Types...>>
    {
        static constexpr std::size_t value = 1 + Tuple_element_index<T, std::tuple<Types...>>::value;
    };


    export template <typename... Component_ts>
    using Vector_tuple = std::tuple<std::vector<Component_ts>...>;
}
