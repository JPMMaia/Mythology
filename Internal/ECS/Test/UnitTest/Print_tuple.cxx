module;

#include <catch2/catch.hpp>

#include <sstream>
#include <tuple>

export module maia.test.print_tuple;

namespace Catch
{
    template<typename... Component_ts>
    struct StringMaker<std::tuple<Component_ts...>>
    {
        static std::string convert(std::tuple<Component_ts...> const& components)
        {
            using Tuple_t = std::tuple<Component_ts...>;
            using Last_component_t = std::tuple_element_t<std::tuple_size_v<Tuple_t> - 1, Tuple_t>; 

            std::stringstream stream;
            stream << '{';
            ((stream << std::get<Component_ts>(components) << (std::is_same_v<Component_ts, Last_component_t> ? "" : ", ")), ...);
            stream << '}';

            return stream.str();
        }
    };
}