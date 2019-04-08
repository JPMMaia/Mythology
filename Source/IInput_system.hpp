#ifndef MAIA_MYTHOLOGY_IINPUT_SYSTEM_H_INCLUDED
#define MAIA_MYTHOLOGY_IINPUT_SYSTEM_H_INCLUDED

namespace Maia::Mythology::Input
{
	struct Input_state;

	class IInput_system
	{
	public:

		virtual Input_state const& execute() = 0;

	};
}

#endif
