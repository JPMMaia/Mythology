add_library (ImGui_module)
add_library (ImGui::Module ALIAS ImGui_module)

set_target_properties (ImGui_module PROPERTIES LINKER_LANGUAGE CXX)

find_package(imgui CONFIG REQUIRED)
target_link_libraries(ImGui_module PUBLIC imgui::imgui)

get_target_property(imgui_INCLUDE_DIRECTORIES imgui::imgui INTERFACE_INCLUDE_DIRECTORIES)

find_file(imgui_h "imgui.h" ${imgui_INCLUDE_DIRECTORIES})
target_header (ImGui_module PUBLIC "ImGui" "${imgui_h}")
