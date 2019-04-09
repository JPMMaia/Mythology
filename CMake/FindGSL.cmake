find_path (GSL_INCLUDE_DIR "gsl/gsl")
mark_as_advanced (GSL_INCLUDE_DIR)

include (FindPackageHandleStandardArgs)

if (NOT GSL_INCLUDE_DIR)

    include (FetchContent)
    FetchContent_Declare (
        GSL
        GIT_REPOSITORY "https://github.com/Microsoft/GSL.git"
        GIT_TAG "master"
    )

    FetchContent_GetProperties (GSL)
    if (NOT gsl_POPULATED)
        
        FetchContent_Populate (GSL)

        find_path (GSL_INCLUDE_DIR "gsl/gsl" HINTS "${gsl_SOURCE_DIR}/include" NO_DEFAULT_PATH)
        find_package_handle_standard_args (GSL REQUIRED_VARS GSL_INCLUDE_DIR)

        if (GSL_FOUND AND NOT TARGET GSL::GSL)
            add_subdirectory (${gsl_SOURCE_DIR} ${gsl_BINARY_DIR})
            add_library (GSL::GSL ALIAS GSL)
        endif()

    else ()

        find_path (GSL_INCLUDE_DIR "gsl/gsl" HINTS "${gsl_SOURCE_DIR}/include" NO_DEFAULT_PATH)
        find_package_handle_standard_args (GSL REQUIRED_VARS GSL_INCLUDE_DIR)

    endif ()

else ()

    find_package_handle_standard_args (GSL REQUIRED_VARS GSL_INCLUDE_DIR)

    if (GSL_FOUND AND NOT TARGET GSL::GSL)

        add_library (GSL::GSL INTERFACE IMPORTED)
        set_target_properties (GSL::GSL PROPERTIES
            INTERFACE_INCLUDE_DIRECTORIES "${GSL_INCLUDE_DIR}"
        )

    endif()

endif()
