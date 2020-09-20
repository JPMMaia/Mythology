add_library (Vulkan)

set_target_properties (Vulkan PROPERTIES LINKER_LANGUAGE CXX)

find_package (Vulkan REQUIRED)
target_link_libraries (Vulkan PUBLIC Vulkan::Vulkan)

target_header (Vulkan PUBLIC "vulkan" "${Vulkan_INCLUDE_DIR}/vulkan/vulkan.h")