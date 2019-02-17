#ifndef MAIA_MYTHOLOGY_WIN32_H_INCLUDED
#define MAIA_MYTHOLOGY_WIN32_H_INCLUDED

#include <IInput_system.hpp>

#include <Input_state.hpp>

namespace Maia::Mythology::Win32
{
	class Input_system final : public Maia::Mythology::Input::IInput_system
	{
	public:

		Maia::Mythology::Input::Input_state const& execute() final;


	private:

		Maia::Mythology::Input::Input_state m_input_state{};

	};
}

#endif