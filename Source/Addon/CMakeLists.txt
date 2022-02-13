add_library(MythologyAddonInterface)
add_library(Mythology::Addon::Interface ALIAS MythologyAddonInterface)

target_compile_features(MythologyAddonInterface PUBLIC cxx_std_23)

target_link_libraries (MythologyAddonInterface PUBLIC Maia::Input)

target_module(
    MythologyAddonInterface
    PUBLIC
    "mythology.addon_interface"
    INTERFACE "Addon_interface.cppm"
)

target_include_directories(MythologyAddonInterface PUBLIC "${CMAKE_CURRENT_LIST_DIR}")
target_sources(MythologyAddonInterface PUBLIC "${CMAKE_CURRENT_LIST_DIR}/Addon_interface.hpp")


add_library(MaiaMythologyAddon)
add_library(Maia::Mythology::Addon ALIAS MaiaMythologyAddon)

target_compile_features(MaiaMythologyAddon PUBLIC cxx_std_23)

target_link_libraries (MaiaMythologyAddon PUBLIC Mythology::Addon::Interface)

target_module(
    MaiaMythologyAddon
    PUBLIC
    "mythology.load_addon"
    INTERFACE "Load_addon.cppm"
    IMPLEMENTATION "Load_addon.cpp"
)
