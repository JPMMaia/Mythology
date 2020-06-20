add_library (MaiaMythologySDL)
add_library (Maia::Mythology::SDL ALIAS MaiaMythologySDL)

target_link_libraries (MaiaMythologySDL PRIVATE Maia::Input)
target_link_libraries (MaiaMythologySDL PRIVATE Maia::Renderer::Vulkan)
target_link_libraries (MaiaMythologySDL PRIVATE Maia::Renderer::Vulkan::Serialization)
target_link_libraries (MaiaMythologySDL PRIVATE Maia::Shaders)
target_link_libraries (MaiaMythologySDL PRIVATE Maia::SDL2)

target_link_libraries (MaiaMythologySDL PRIVATE Maia::Mythology::ImGui)
target_link_libraries (MaiaMythologySDL PRIVATE Maia::Mythology::Core)

target_link_libraries (MaiaMythologySDL PRIVATE ImGui::Module)
target_link_libraries (MaiaMythologySDL PRIVATE nlohmann_json_module)
target_link_libraries (MaiaMythologySDL PRIVATE Standard_library)
target_link_libraries (MaiaMythologySDL PRIVATE Vulkan)

if (MaiaRenderer_build_vulkan)
    target_module (
        MaiaMythologySDL
        "maia.sdl.vulkan"
        INTERFACE "Vulkan_SDL.cxx"
        INTERFACE_DEPENDS "ImGui" "SDL" "Standard_library" "Vulkan"
        IMPLEMENTATION "Vulkan_SDL.cpp"
        IMPLEMENTATION_DEPENDS "ImGui" "SDL" "SDL_vulkan" "Standard_library" "Vulkan"
    )
endif ()

target_module (
    MaiaMythologySDL
    "mythology.sdl.application"
    INTERFACE "Application.cxx"
    INTERFACE_DEPENDS "Standard_library" "nlohmann_json_module"
    IMPLEMENTATION "Application.cpp"
    IMPLEMENTATION_DEPENDS "maia.input" "maia.sdl.vulkan" "mythology.core.vulkan" "mythology.core.utilities" "SDL" "SDL_vulkan" "Standard_library" "nlohmann_json_module" "maia.renderer.vulkan.serializer"
)