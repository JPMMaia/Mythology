#ifndef MAIA_MYTHOLOGY_INPUTSYSTEM_H_INCLUDED
#define MAIA_MYTHOLOGY_INPUTSYSTEM_H_INCLUDED

#include <bitset>
#include <cstdint>

#include <Eigen/Core>

#include <winrt/Windows.UI.Core.h>

namespace Maia::Mythology::Input
{
	struct Key
	{
		 winrt::Windows::System::VirtualKey value{};
	};

	struct Keys_state
	{
		std::bitset<256> value{};

		bool is_down(Key key) const
		{
			return this->value.test(static_cast<std::size_t>(key.value));
		}

		bool is_up(Key key) const
		{
			return !is_down(key);
		}

		void set(Key key, bool value)
		{
			this->value.set(static_cast<std::size_t>(key.value), value);
		}
	};

	struct Mouse_state
	{
		Eigen::Vector2f position{ 0.0f, 0.0f };
	};
	
	struct Input_state
	{
		Keys_state keys_previous_state{};
		Keys_state keys_current_state{};
		Mouse_state mouse_previous_state{};
		Mouse_state mouse_current_state{};

		bool is_down(Key key) const
		{
			return this->keys_current_state.is_down(key);
		}
		bool is_up(Key key) const
		{
			return this->keys_current_state.is_up(key);
		}
		bool is_pressed(Key key) const
		{
			return this->keys_current_state.is_down(key) 
				&& this->keys_previous_state.is_up(key);
		}
		bool is_released(Key key) const
		{
			return this->keys_previous_state.is_down(key) 
				&& this->keys_current_state.is_up(key);
		}

		Eigen::Vector2f delta_mouse_position() const
		{
			return this->mouse_current_state.position - this->mouse_previous_state.position;
		}

		void set(Key key, bool value)
		{
			this->keys_current_state.set(key, value);
		}

		void set(Eigen::Vector2f mouse_position)
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
