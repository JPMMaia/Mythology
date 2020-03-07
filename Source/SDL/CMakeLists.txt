add_library (MaiaMythologySDL)
add_library (Maia::Mythology::SDL ALIAS MaiaMythologySDL)

target_link_libraries (MaiaMythologySDL PRIVATE Maia::Input)
target_link_libraries (MaiaMythologySDL PRIVATE Maia::Renderer::Vulkan)
target_link_libraries (MaiaMythologySDL PRIVATE Maia::SDL2)

target_link_libraries (MaiaMythologySDL PRIVATE Maia::Mythology::Core)

target_link_libraries (MaiaMythologySDL PRIVATE Standard_library)
target_link_libraries (MaiaMythologySDL PRIVATE Vulkan)

if (MaiaRenderer_build_vulkan)
    target_module (
        MaiaMythologySDL
        "maia.sdl.vulkan"
        INTERFACE "Vulkan_SDL.cxx"
        INTERFACE_DEPENDS "SDL" "Standard_library" "Vulkan"
        IMPLEMENTATION "Vulkan_SDL.cpp"
        IMPLEMENTATION_DEPENDS "SDL" "SDL_vulkan" "Standard_library" "Vulkan"
    )
endif ()

target_module (
    MaiaMythologySDL
    "mythology.sdl.application"
    INTERFACE "Application.cxx"
    INTERFACE_DEPENDS "Standard_library"
    IMPLEMENTATION "Application.cpp"
    IMPLEMENTATION_DEPENDS "maia.input" "maia.sdl.vulkan" "SDL" "SDL_vulkan" "Standard_library"
)