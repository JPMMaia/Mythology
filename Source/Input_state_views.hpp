#ifndef MAIA_MYTHOLOGY_INPUTSTATEVIEWS_H_INCLUDED
#define MAIA_MYTHOLOGY_INPUTSTATEVIEWS_H_INCLUDED

#include <cstdint>

#include <Eigen/Core>

namespace Maia::Mythology
{
	enum class Game_key : std::uint8_t;
}

namespace Maia::Mythology::Input
{
	struct Input_state;

	class Input_events_view
	{
	public:

		explicit Input_events_view(Input_state const& input_state) noexcept;


		bool is_pressed(Game_key key) const;
		bool is_released(Game_key key) const;


	private:

		Input_state const& m_input_state;

	};

	class Input_state_view
	{
	public:

		explicit Input_state_view(Input_state const& input_state) noexcept;


		bool is_down(Game_key key) const;
		bool is_up(Game_key key) const;

		Eigen::Vector2i delta_mouse_position() const;


	private:

		Input_state const& m_input_state;

	};
}

#endif
