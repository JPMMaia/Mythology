#ifndef MAIA_GAMEENGINE_TEST_H_INCLUDED
#define MAIA_GAMEENGINE_TEST_H_INCLUDED

#include <ostream>

#include <Maia/GameEngine/Component.hpp>

namespace Maia::GameEngine::Test
{
	struct Position
	{
		float x, y, z;
	};

	inline bool operator==(Position const& lhs, Position const& rhs)
	{
		return lhs.x == rhs.x
			&& lhs.y == rhs.y
			&& lhs.z == rhs.z;
	}

	inline std::ostream& operator<<(std::ostream& output_stream, Position const& value)
	{
		output_stream << "{" << value.x << ", " << value.y << ", " << value.z << "}";
		return output_stream;
	}

	struct Rotation
	{
		float a, b, c, w;
	};

	inline bool operator==(Rotation const& lhs, Rotation const& rhs)
	{
		return lhs.a == rhs.a
			&& lhs.b == rhs.b
			&& lhs.c == rhs.c
			&& lhs.w == rhs.w;
	}

	inline std::ostream& operator<<(std::ostream& output_stream, Rotation const& value)
	{
		output_stream << "{" << value.a << ", " << value.b << ", " << value.c << ", " << value.w << "}";
		return output_stream;
	}
}

#endif