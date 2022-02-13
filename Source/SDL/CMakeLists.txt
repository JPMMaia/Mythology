add_library(MaiaMythologySDL)
add_library(Maia::Mythology::SDL ALIAS MaiaMythologySDL)

find_package(nlohmann_json REQUIRED)
target_link_libraries(MaiaMythologySDL PRIVATE nlohmann_json::nlohmann_json)

find_package(SDL2 REQUIRED)
target_link_libraries(MaiaMythologySDL PRIVATE SDL2::SDL2)

target_link_libraries(MaiaMythologySDL PRIVATE Vulkan Maia::glTF)
target_link_libraries(MaiaMythologySDL PRIVATE Vulkan Maia::Renderer::Vulkan)
target_link_libraries(MaiaMythologySDL PRIVATE Vulkan Maia::Scene)
target_link_libraries(MaiaMythologySDL PRIVATE Vulkan Maia::Mythology::Addon)

find_package(klein 2.3.0 REQUIRED)
target_link_libraries(MaiaMythologySDL PRIVATE klein::klein_sse42)

target_compile_features(MaiaMythologySDL PUBLIC cxx_std_23)

target_link_libraries (MaiaMythologySDL PRIVATE Maia::Input)
#target_link_libraries (MaiaMythologySDL PRIVATE Maia::Renderer::Vulkan::Serialization)
#target_link_libraries (MaiaMythologySDL PRIVATE Maia::Shaders)
#target_link_libraries (MaiaMythologySDL PRIVATE Maia::Utilities::GLTF)
#target_link_libraries (MaiaMythologySDL PRIVATE Maia::SDL2)

#target_link_libraries (MaiaMythologySDL PRIVATE Maia::Mythology::ImGui)
#target_link_libraries (MaiaMythologySDL PRIVATE Maia::Mythology::Core)

#target_link_libraries (MaiaMythologySDL PRIVATE ImGui::Module)
#target_link_libraries (MaiaMythologySDL PRIVATE nlohmann_json_module)
#target_link_libraries (MaiaMythologySDL PRIVATE Standard_library)
#target_link_libraries (MaiaMythologySDL PRIVATE Vulkan)

#[[
if (MaiaRenderer_build_vulkan)
    target_module (
        MaiaMythologySDL
        PUBLIC
        "maia.sdl.vulkan"
        INTERFACE "Vulkan_SDL.cppm"
        INTERFACE_DEPENDS "ImGui" "SDL" "Standard_library" "Vulkan"
        IMPLEMENTATION "Vulkan_SDL.cpp"
        IMPLEMENTATION_DEPENDS "ImGui" "SDL" "SDL_vulkan" "Standard_library" "Vulkan"
    )
endif ()
#]]
#[[
target_module (
    MaiaMythologySDL
    PUBLIC
    "mythology.sdl.application"
    INTERFACE "Application.cppm"
    INTERFACE_DEPENDS "Standard_library" "nlohmann_json_module"
    IMPLEMENTATION "Application.cpp"
    IMPLEMENTATION_DEPENDS "maia.input" "maia.sdl.vulkan" "mythology.core.vulkan" "mythology.core.utilities" "SDL" "SDL_vulkan" "Standard_library" "nlohmann_json_module" "maia.renderer.vulkan.serializer"
)
#]]

target_module(
    MaiaMythologySDL
    PUBLIC
    "mythology.geometry"
    INTERFACE "Geometry.cppm"
    IMPLEMENTATION "Geometry.cpp"
)

target_module(
    MaiaMythologySDL
    PUBLIC
    "mythology.sdl.sdl"
    INTERFACE "SDL.cppm"
    IMPLEMENTATION "SDL.cpp"
)

target_module (
    MaiaMythologySDL
    PUBLIC
    "mythology.sdl.vulkan"
    INTERFACE "Vulkan_SDL.cppm"
    IMPLEMENTATION "Vulkan_SDL.cpp"
)

target_module(
    MaiaMythologySDL
    PUBLIC
    "mythology.sdl.render_resources"
    INTERFACE "Render_resources.cppm"
    IMPLEMENTATION "Render_resources.cpp"
)

target_module(
    MaiaMythologySDL
    PUBLIC
    "mythology.sdl.configuration"
    INTERFACE "Configuration.cppm"
    INTERFACE_DEPENDS
        "mythology.sdl.render_resources"
        "mythology.sdl.sdl"
    IMPLEMENTATION "Configuration.cpp"
    IMPLEMENTATION_DEPENDS
        "mythology.sdl.render_resources"
        "mythology.sdl.sdl"
        "mythology.sdl.vulkan"
)

target_module(
    MaiaMythologySDL
    PUBLIC
    "mythology.sdl.state"
    INTERFACE "State.cppm"
)

target_module(
    MaiaMythologySDL
    PUBLIC
    "mythology.sdl.startup_state"
    INTERFACE "Startup_state.cppm"
    INTERFACE_DEPENDS "mythology.sdl.state"
    IMPLEMENTATION "Startup_state.cpp"
    IMPLEMENTATION_DEPENDS
        "mythology.sdl.configuration"
        "mythology.sdl.render_resources"
        "mythology.sdl.sdl"
        "mythology.sdl.state"
        "mythology.sdl.vulkan"
)

target_module(
    MaiaMythologySDL
    PUBLIC
    "mythology.sdl.application_2"
    INTERFACE "Application_2.cppm"
    IMPLEMENTATION "Application_2.cpp"
    IMPLEMENTATION_DEPENDS "mythology.sdl.state" "mythology.sdl.startup_state"
)
