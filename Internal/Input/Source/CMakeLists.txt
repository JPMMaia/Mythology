project (MaiaInput VERSION 0.0.1)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/../../Utilities/CMake/Modules")
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/../../Utilities/CMake/Scripts")

add_library (MaiaInput)
add_library (Maia::Input ALIAS MaiaInput)

set_target_properties (MaiaInput PROPERTIES LINKER_LANGUAGE CXX)

set_target_properties (MaiaInput PROPERTIES 
	LINKER_LANGUAGE "CXX"
	CXX_STANDARD 20
	CXX_STANDARD_REQUIRED ON
)

target_module (
    MaiaInput
    PUBLIC
    "maia.input.game_controller_state"
    INTERFACE "Game_controller_state.cppm"
    INTERFACE_DEPENDS "Standard_library"
)

target_module (
    MaiaInput
    PUBLIC
    "maia.input.input_mapping"
    INTERFACE "Input_mapping.cppm"
    INTERFACE_DEPENDS
        "maia.input.game_controller_state"
        "maia.input.input_state"
        "maia.input.keyboard_state"
        "maia.input.mouse_state"
)

target_module (
    MaiaInput
    PUBLIC
    "maia.input.input_state"
    INTERFACE "Input_state.cppm"
    INTERFACE_DEPENDS "Standard_library"
)

target_module (
    MaiaInput
    PUBLIC
    "maia.input.keyboard_state"
    INTERFACE "Keyboard_state.cppm"
    INTERFACE_DEPENDS "Standard_library"
)

target_module (
    MaiaInput
    PUBLIC
    "maia.input.mouse_state"
    INTERFACE "Mouse_state.cppm"
    INTERFACE_DEPENDS "Standard_library"
)

target_module (
    MaiaInput
    PUBLIC
    "maia.input"
    INTERFACE "Module.cppm"
    INTERFACE_DEPENDS    
        "maia.input.game_controller_state"
        "maia.input.input_mapping"
        "maia.input.input_state"
        "maia.input.keyboard_state"
        "maia.input.mouse_state"
)