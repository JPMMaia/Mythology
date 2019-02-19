#ifndef MAIA_MYTHOLOGY_INPUTSTATE_H_INCLUDED
#define MAIA_MYTHOLOGY_INPUTSTATE_H_INCLUDED

#include <bitset>
#include <cstdint>

#include <Eigen/Core>

namespace Maia::Mythology
{
	enum class Game_key : std::uint8_t;
}

namespace Maia::Mythology::Input
{
	struct Keys_state
	{
		std::bitset<256> value{};

		bool is_down(Game_key key) const
		{
			return this->value.test(static_cast<std::size_t>(key));
		}

		bool is_up(Game_key key) const
		{
			return !is_down(key);
		}

		void set(Game_key key, bool value)
		{
			this->value.set(static_cast<std::size_t>(key), value);
		}
	};

	struct Mouse_state
	{
		Eigen::Vector2i position{ 0, 0 };
		Eigen::Vector2i delta{ 0, 0 };
	};
	
	struct Input_state
	{
		Keys_state keys_previous_state{};
		Keys_state keys_current_state{};
		Mouse_state mouse_previous_state{};
		Mouse_state mouse_current_state{};


		bool is_down(Game_key key) const
		{
			return this->keys_current_state.is_down(key);
		}
		bool is_up(Game_key key) const
		{
			return this->keys_current_state.is_up(key);
		}
		
		bool is_pressed(Game_key key) const
		{
			return this->keys_current_state.is_down(key) 
				&& this->keys_previous_state.is_up(key);
		}
		bool is_released(Game_key key) const
		{
			return this->keys_previous_state.is_down(key) 
				&& this->keys_current_state.is_up(key);
		}

		Eigen::Vector2i delta_mouse_position() const
		{
			return this->mouse_current_state.position - this->mouse_previous_state.position;
		}

		void set(Game_key key, bool value)
		{
			this->keys_current_state.set(key, value);
		}

		void set(Eigen::Vector2i mouse_position)
		{
			this->mouse_current_state.position = mouse_position;
		}

		void overwrite_previous_with_current_state()
		{
			this->keys_previous_state = this->keys_current_state;
			this->mouse_previous_state = this->mouse_current_state;
		}
	};
}

#endif
