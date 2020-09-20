export module maia.ecs.archetype;

import <concepts>;

namespace Maia::ECS
{
    namespace Concept
    {
        export template<typename T>
        concept Component = std::regular<T>;

        export template<typename T>
        concept Shared_component = std::movable<T>;
    }

    export template<Concept::Component... T>
    struct Components_tag{};

    export template<Concept::Shared_component T>
    struct Shared_component_tag{};

    template<typename T, typename U = void>
    struct Archetype;

    export template<
        template<typename...> typename Components_tag,
        Concept::Component... Components
    >
    struct Archetype<Components_tag<Components...>>
    {
        static constexpr bool has_shared_component() noexcept
        {
            return false;
        }

        static constexpr unsigned int get_component_count() noexcept
        {
            return sizeof...(Components);
        }
    };

    export template<
        template<typename> typename Shared_component_tag,
        template<typename...> typename Components_tag,
        Concept::Shared_component Shared_component_t,
        Concept::Component... Components
        //Shared_component_tag<Concept::Shared_component Shared_component_t> S,
        //Components_tag<Concept::Component... Components> C
    >
    struct Archetype<Shared_component_tag<Shared_component_t>, Components_tag<Components...>>
    {
        using Shared_component = Shared_component_t;

        static constexpr bool has_shared_component() noexcept
        {
            return true;
        }

        static constexpr unsigned int get_component_count() noexcept
        {
            return sizeof...(Components);
        }
    };
}
