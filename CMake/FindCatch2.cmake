include (FetchContent)
FetchContent_Declare (
    Catch2
    URL "https://github.com/catchorg/Catch2/archive/v2.4.1.zip"
)

FetchContent_GetProperties (Catch2)
if (NOT catch2_POPULATED)
    
    FetchContent_Populate (Catch2)

    find_path (catch2_INCLUDE_DIR "catch2/catch.hpp" HINTS "${catch2_SOURCE_DIR}/single_include" NO_DEFAULT_PATH)

    include (FindPackageHandleStandardArgs)
    find_package_handle_standard_args (Catch2 REQUIRED_VARS catch2_INCLUDE_DIR)

    if (Catch2_FOUND AND NOT TARGET Catch2::Catch2)
        
        add_subdirectory ("${catch2_SOURCE_DIR}" "${catch2_BINARY_DIR}")

        list (APPEND CMAKE_MODULE_PATH "${catch2_SOURCE_DIR}/contrib")

    endif()

else ()

    find_path (catch2_INCLUDE_DIR "catch2/catch.hpp" HINTS "${catch2_SOURCE_DIR}/single_include" NO_DEFAULT_PATH)

    include (FindPackageHandleStandardArgs)
    find_package_handle_standard_args (Catch2 REQUIRED_VARS catch2_INCLUDE_DIR)

    list (APPEND CMAKE_MODULE_PATH "${catch2_SOURCE_DIR}/contrib")

endif ()
