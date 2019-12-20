find_path (MICROSOFTGSL_INCLUDE_DIR "gsl/gsl")
mark_as_advanced (MICROSOFTGSL_INCLUDE_DIR)

include (FindPackageHandleStandardArgs)
find_package_handle_standard_args (MICROSOFTGSL REQUIRED_VARS MICROSOFTGSL_INCLUDE_DIR)

if (MICROSOFTGSL_FOUND AND NOT TARGET MicrosoftGSL::MicrosoftGSL)

    add_library (MicrosoftGSL::MicrosoftGSL INTERFACE IMPORTED)
    set_target_properties (MicrosoftGSL::MicrosoftGSL PROPERTIES
        INTERFACE_INCLUDE_DIRECTORIES "${MICROSOFTGSL_INCLUDE_DIR}"
    )

endif()
