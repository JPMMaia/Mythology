add_library(Vulkan)

set_target_properties(Vulkan PROPERTIES LINKER_LANGUAGE CXX)

find_package(Vulkan REQUIRED)
target_link_libraries(Vulkan PUBLIC Vulkan::Vulkan)

target_compile_definitions(
    Vulkan
        PUBLIC
            "VULKAN_HPP_DISPATCH_LOADER_DYNAMIC"
            "VULKAN_HPP_NO_STRUCT_CONSTRUCTORS"
)

target_header(Vulkan PUBLIC "vulkan" "${Vulkan_INCLUDE_DIR}/vulkan/vulkan.hpp")

target_sources(Vulkan PRIVATE "Vulkan.cpp")
