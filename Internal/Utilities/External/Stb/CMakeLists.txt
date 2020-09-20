find_package (Stb REQUIRED)

add_library (Stb_module)

target_include_directories (Stb_module PUBLIC "${Stb_INCLUDE_DIR}")

set_target_properties (Stb_module PROPERTIES LINKER_LANGUAGE CXX)

target_header (Stb_module PUBLIC "Stb_image" "${Stb_INCLUDE_DIR}/stb_image.h")
target_header (Stb_module PUBLIC "Stb_image_write" "${Stb_INCLUDE_DIR}/stb_image_write.h")

target_sources (
    Stb_module
        PRIVATE
            "stb_image.cpp"
)
