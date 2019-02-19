#include <Input_state.hpp>

#include "Input_state_views.hpp"

namespace Maia::Mythology::Input
{
	Input_events_view::Input_events_view(Input_state const& input_state) noexcept :
		m_input_state{ input_state }
	{
	}


	bool Input_events_view::is_pressed(Game_key key) const
	{
		return m_input_state.is_pressed(key);
	}

	bool Input_events_view::is_released(Game_key key) const
	{
		return m_input_state.is_released(key);
	}



	Input_state_view::Input_state_view(Input_state const& input_state) noexcept :
		m_input_state{ input_state }
	{
	}


	bool Input_state_view::is_down(Game_key key) const
	{
		return m_input_state.is_down(key);
	}

	bool Input_state_view::is_up(Game_key key) const
	{
		return m_input_state.is_up(key);
	}

	Eigen::Vector2i Input_state_view::delta_mouse_position() const
	{
		return m_input_state.delta_mouse_position();
	}
}
