add_library (MaiaMythologyImGui)
add_library (Maia::Mythology::ImGui ALIAS MaiaMythologyImGui)

target_link_libraries (MaiaMythologyImGui PRIVATE Maia::Renderer::Vulkan)

target_link_libraries (MaiaMythologyImGui PRIVATE ImGui::Module)
target_link_libraries (MaiaMythologyImGui PRIVATE Standard_library)
target_link_libraries (MaiaMythologyImGui PRIVATE Vulkan)

target_module (
    MaiaMythologyImGui
    PUBLIC
    "mythology.imgui"
    INTERFACE "ImGui.cppm"
    INTERFACE_DEPENDS "ImGui" "Standard_library" "Vulkan" "maia.renderer.vulkan"
    IMPLEMENTATION "ImGui.cpp"
    IMPLEMENTATION_DEPENDS "ImGui" "Standard_library" "Vulkan" "maia.renderer.vulkan"
)
