add_library (MaiaSDL2)
add_library (Maia::SDL2 ALIAS MaiaSDL2)

set_target_properties (MaiaSDL2 PROPERTIES LINKER_LANGUAGE CXX)

find_package (SDL2 CONFIG REQUIRED)
target_link_libraries (MaiaSDL2 PUBLIC SDL2::SDL2)

get_target_property(SDL2_INCLUDE_DIRECTORIES SDL2::SDL2 INTERFACE_INCLUDE_DIRECTORIES)

find_file(SDL_h "SDL2/SDL.h" ${SDL2_INCLUDE_DIRECTORIES})
target_header (MaiaSDL2 PUBLIC "SDL" "${SDL_h}" COMPILE_OPTIONS "-DSDL_MAIN_HANDLED")

find_file(SDL_vulkan_h "SDL_vulkan.h" ${SDL2_INCLUDE_DIRECTORIES})
target_header (MaiaSDL2 PUBLIC "SDL_vulkan" "${SDL_vulkan_h}" COMPILE_OPTIONS "-DSDL_MAIN_HANDLED")