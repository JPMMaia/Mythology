module;

#include <cassert>
#include <cstddef>
#include <memory_resource>

module maia.scene;

namespace Maia::Scene
{
    std::uint8_t size_of(Component_type const component_type) noexcept
    {
        switch (component_type)
		{
		case Component_type::Byte:
		case Component_type::Unsigned_byte:
			return 1;
		case Component_type::Short:
		case Component_type::Unsigned_short:
			return 2;
		case Component_type::Unsigned_int:
		case Component_type::Float:
			return 4;
		default:
            assert(false);
            return 1;
		}
    }

    std::uint8_t size_of(Accessor::Type const accessor_type) noexcept
    {
        switch (accessor_type)
		{
		case Accessor::Type::Scalar: return 1;
		case Accessor::Type::Vector2: return 2;
		case Accessor::Type::Vector3: return 3;
		case Accessor::Type::Vector4: return 4;
		case Accessor::Type::Matrix2x2: return 4;
		case Accessor::Type::Matrix3x3: return 9;
		case Accessor::Type::Matrix4x4: return 16;
		default:
            assert(false);
            return 1;
		}
    }


    bool operator==(Attribute const lhs, Attribute const rhs) noexcept
    {
        return lhs.type == rhs.type && lhs.index == rhs.index;
    }

    std::size_t Attribute_hash::operator() (Attribute const attribute) const noexcept
    {
        constexpr std::size_t maximum_index = 100;
        assert(attribute.index < maximum_index);

        std::size_t const type = static_cast<std::size_t>(attribute.type);
        Index const index = attribute.index;

        return type*maximum_index + index;
    }
}
