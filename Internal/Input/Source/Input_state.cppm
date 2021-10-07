export module maia.input.input_state;

import <array>;
import <cstdint>;

namespace Maia::Input
{
    export struct Key_id
    {
        std::uint8_t value;

        static constexpr Key_id unused_value() noexcept
        {
            return {std::numeric_limits<std::uint8_t>::max()};
        }

        constexpr bool is_valid() const noexcept
        {
            return this->value != unused_value().value;
        }
    };

    export struct Axis_id
    {
        std::uint8_t value;

        static constexpr Axis_id unused_value() noexcept
        {
            return {std::numeric_limits<std::uint8_t>::max()};
        }

        constexpr bool is_valid() const noexcept
        {
            return this->value != unused_value().value;
        }
    };

    export struct Trigger_id
    {
        std::uint8_t value;

        static constexpr Trigger_id unused_value() noexcept
        {
            return {std::numeric_limits<std::uint8_t>::max()};
        }

        constexpr bool is_valid() const noexcept
        {
            return this->value != unused_value().value;
        }
    };

    export struct Axis
    {
        using value_type = std::int16_t;

        value_type value;

        static constexpr Axis maximum() noexcept
        {
            return {std::numeric_limits<value_type>::max()};
        }

        static constexpr Axis minimum() noexcept
        {
            return {std::numeric_limits<value_type>::lowest()};
        }

        template <class T>
        constexpr T normalized() const noexcept
        {
            return this->value >= 0 ? 
                T{static_cast<T>(this->value) / maximum().value} :
                T{-static_cast<T>(this->value) / minimum().value};
        }
    };

    export struct Trigger
    {
        using value_type = std::int16_t;

        value_type value;

        static constexpr Axis maximum() noexcept
        {
            return {std::numeric_limits<value_type>::max()};
        }

        static constexpr Axis minimum() noexcept
        {
            return {};
        }

        template <class T>
        constexpr T normalized() const noexcept
        {
            return this->value >= 0 ? 
                T{static_cast<T>(this->value) / maximum().value} :
                T{-static_cast<T>(this->value) / minimum().value};
        }
    };

    export struct Input_state
    {
        std::array<bool, 256> keys;
        std::array<Axis, 8> axis;
        std::array<Trigger, 2> triggers;

        bool is_down(Key_id const key_id) const noexcept
		{
			return this->keys[key_id.value];
		}

		bool is_up(Key_id const key_id) const noexcept
		{
			return !is_down(key_id);
		}

        Axis axis_value(Axis_id const axis_id) const noexcept
        {
            return this->axis[axis_id.value];
        }

        Trigger trigger_value(Trigger_id const trigger_id) const noexcept
        {
            return this->triggers[trigger_id.value];
        }
    };

    export bool is_pressed(
		Key_id const key,
		Input_state const& previous_input_state,
		Input_state const& current_input_state) noexcept
	{
		return current_input_state.is_down(key) 
			&& previous_input_state.is_up(key);
	}
	
	export bool is_released(
		Key_id const key,
		Input_state const& previous_input_state,
		Input_state const& current_input_state) noexcept
	{
		return previous_input_state.is_down(key) 
			&& current_input_state.is_up(key);
	}
}
