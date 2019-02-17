#ifndef MAIA_MYTHOLOGY_GAMEKEY_H_INCLUDED
#define MAIA_MYTHOLOGY_GAMEKEY_H_INCLUDED

#include <cstdint>

namespace Maia::Mythology
{
	enum class Game_key : std::uint8_t
	{
		None = 0,
		Move_left = 1,
		Move_forward = 2,
		Move_right = 3,
		Move_back = 4,
	};
}

#endif