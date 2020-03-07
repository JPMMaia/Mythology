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
    "maia.input.keyboard_state"
    INTERFACE "Keyboard_state.cxx"
    INTERFACE_DEPENDS "Standard_library"
)

target_module (
    MaiaInput
    "maia.input.mouse_state"
    INTERFACE "Mouse_state.cxx"
    INTERFACE_DEPENDS "Standard_library"
)

target_module (
    MaiaInput
    "maia.input"
    INTERFACE "Module.cxx"
    INTERFACE_DEPENDS
        "maia.input.keyboard_state"
        "maia.input.mouse_state"
)