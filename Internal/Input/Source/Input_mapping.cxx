export module maia.input.input_mapping;

import maia.input.game_controller_state;
import maia.input.input_state;
import maia.input.keyboard_state;
import maia.input.mouse_state;

import <array>;
import <cassert>;
import <cstdint>;
import <iterator>;
import <span>;
import <variant>;

namespace Maia::Input
{
    export struct Positive_axis_id
    {
        std::uint8_t value;

        static constexpr Positive_axis_id unused_value() noexcept
        {
            return {std::numeric_limits<std::uint8_t>::max()};
        }

        constexpr bool is_valid() const noexcept
        {
            return this->value != unused_value().value;
        }
    };

    export struct Negative_axis_id
    {
        std::uint8_t value;

        static constexpr Negative_axis_id unused_value() noexcept
        {
            return {std::numeric_limits<std::uint8_t>::max()};
        }

        constexpr bool is_valid() const noexcept
        {
            return this->value != unused_value().value;
        }
    };

    export template <std::size_t count>
    struct Keyboard_mapping
    {
        using Key_type = Keyboard_key;
        using Value_type = std::variant<Key_id, Positive_axis_id, Negative_axis_id, Trigger_id>;

        std::array<Key_type, count> keys;
        std::array<Value_type, count> values;
    };

    export template <std::size_t count>
    using Keyboard_mapping_key_value_pair = std::pair<typename Keyboard_mapping<count>::Key_type, typename Keyboard_mapping<count>::Value_type>;

    export template <std::size_t count>
    constexpr Keyboard_mapping<count> create_keyboard_mapping(
        std::initializer_list<Keyboard_mapping_key_value_pair<count>> const key_value_pairs) noexcept
    {
        Keyboard_mapping<count> keyboard_mapping{};

        for (auto key_value_it = key_value_pairs.begin(); key_value_it != key_value_pairs.end(); ++key_value_it)
        {
            auto const key_index = std::distance(key_value_pairs.begin(), key_value_it);

            keyboard_mapping.keys[key_index] = key_value_it->first;
            keyboard_mapping.values[key_index] = key_value_it->second;
        }

        return keyboard_mapping;
    }


    template <std::size_t size, class T>
    constexpr std::array<T, size> generate_unused_array(T const value) noexcept
    {
        std::array<T, size> array;
        
        for (std::size_t i = 0; i < array.size(); ++i)
        {
            array[i] = value;
        }
        
        return array;
    }

    export struct Mouse_mapping
    {
        std::array<Key_id, 8> keys = generate_unused_array<8>(Key_id::unused_value());
        Axis_id horizontal_axis = Axis_id::unused_value();
        Axis_id vertical_axis = Axis_id::unused_value();
    };

    export struct Game_controller_mapping
    {
        std::array<Key_id, 15> buttons = generate_unused_array<15>(Key_id::unused_value());
        Axis_id horizontal_left_axis = Axis_id::unused_value();
        Axis_id vertical_left_axis = Axis_id::unused_value();
        Axis_id horizontal_right_axis = Axis_id::unused_value();
        Axis_id vertical_right_axis = Axis_id::unused_value();
        Trigger_id left_trigger = Trigger_id::unused_value();
        Trigger_id right_trigger = Trigger_id::unused_value();
    };

    export template <std::size_t size, class From, class To>
    constexpr std::array<To, size> create_array_mapping(
        std::initializer_list<std::pair<From, To> const> const from_to_pairs) noexcept
    {
        std::array<To, size> mapping = generate_unused_array<size>(To::unused_value());

        for (std::pair<From, To> const from_to_pair : from_to_pairs)
        {
            mapping[from_to_pair.first.value] = from_to_pair.second;
        }

        return mapping;
    }

    export template <std::size_t Keyboard_count>
    Input_state map_input(
        std::span<Game_controller_state const> const& game_controller_states,
        std::span<Game_controller_mapping const> const& game_controller_mappings,
        Keyboard_state const& keyboard_state,
        Keyboard_mapping<Keyboard_count> const& keyboard_mapping,
        Mouse_state const& mouse_state,
        Mouse_mapping const& mouse_mapping) noexcept
    {
        assert(game_controller_states.size() <= game_controller_mappings.size());

        Input_state input_state{};

        for (std::size_t game_controller_index = 0; game_controller_index < game_controller_states.size(); ++game_controller_index)
        {
            Game_controller_state const& game_controller_state = game_controller_states[game_controller_index];
            Game_controller_mapping const& game_controller_mapping = game_controller_mappings[game_controller_index];

            for (std::size_t button_index = 0; button_index < game_controller_mapping.buttons.size(); ++button_index)
            {
                bool const button_down = game_controller_state.buttons[button_index];

                if (button_down)
                {
                    Key_id const key_id = game_controller_mapping.buttons[button_index];
                    input_state.keys[key_id.value] |= button_down;
                }
            }

            if (game_controller_mapping.horizontal_left_axis.is_valid())
            {
                input_state.axis[game_controller_mapping.horizontal_left_axis.value] = {game_controller_state.left_axis.x};
            }

            if (game_controller_mapping.vertical_left_axis.is_valid())
            {
                input_state.axis[game_controller_mapping.vertical_left_axis.value] = {game_controller_state.left_axis.y};
            }

            if (game_controller_mapping.horizontal_right_axis.is_valid())
            {
                input_state.axis[game_controller_mapping.horizontal_right_axis.value] = {game_controller_state.right_axis.x};
            }

            if (game_controller_mapping.vertical_right_axis.is_valid())
            {
                input_state.axis[game_controller_mapping.vertical_right_axis.value] = {game_controller_state.right_axis.y};
            }

            if (game_controller_mapping.left_trigger.is_valid())
            {
                input_state.triggers[game_controller_mapping.left_trigger.value] = {game_controller_state.left_trigger.value};
            }

            if (game_controller_mapping.right_trigger.is_valid())
            {
                input_state.triggers[game_controller_mapping.right_trigger.value] = {game_controller_state.right_trigger.value};
            }
        }

        for (std::size_t key_index = 0; key_index < Keyboard_count; ++key_index)
        {
            Keyboard_key const key = keyboard_mapping.keys[key_index];
            bool const key_down = keyboard_state.keys[key.value];

            if (key_down)
            {
                std::variant<Key_id, Positive_axis_id, Negative_axis_id, Trigger_id> const input_element = keyboard_mapping.values[key_index];

                if (std::holds_alternative<Key_id>(input_element))
                {
                    Key_id const key_id = std::get<Key_id>(input_element);
                    input_state.keys[key_id.value] |= key_down;
                }
                else if (std::holds_alternative<Positive_axis_id>(input_element))
                {
                    Positive_axis_id const positive_axis_id = std::get<Positive_axis_id>(input_element);
                    input_state.axis[positive_axis_id.value] = {std::numeric_limits<std::int16_t>::max()};
                }
                else if (std::holds_alternative<Negative_axis_id>(input_element))
                {
                    Negative_axis_id const negative_axis_id = std::get<Negative_axis_id>(input_element);
                    input_state.axis[negative_axis_id.value] = {std::numeric_limits<std::int16_t>::lowest()};
                }
                else if (std::holds_alternative<Trigger_id>(input_element))
                {
                    Trigger_id const trigger_id = std::get<Trigger_id>(input_element);
                    input_state.triggers[trigger_id.value] = {std::numeric_limits<std::int16_t>::max()};
                }
            }
        }

        {
            assert(mouse_mapping.keys.size() == mouse_state.keys.size());

            for (std::size_t key_index = 0; key_index < mouse_mapping.keys.size(); ++key_index)
            {
                bool const key_down = mouse_state.keys[key_index];

                if (key_down)
                {
                    Key_id const key_id = mouse_mapping.keys[key_index];
                    input_state.keys[key_id.value] |= key_down;
                }
            }

            if (mouse_mapping.horizontal_axis.is_valid())
            {
                // TODO
            }

            if (mouse_mapping.vertical_axis.is_valid())
            {
                // TODO
            }
        }

        return input_state;
    }
}
