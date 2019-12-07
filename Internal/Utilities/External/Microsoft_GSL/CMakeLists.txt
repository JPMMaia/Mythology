find_package (MicrosoftGSL REQUIRED)
get_target_property(MicrosoftGSL_include_directories MicrosoftGSL::MicrosoftGSL INTERFACE_INCLUDE_DIRECTORIES)
find_path(MicrosoftGSL_include_directory "gsl/span" PATHS ${MicrosoftGSL_include_directories})

add_module_header (
    "gsl.span" "${MicrosoftGSL_include_directory}/gsl/span" 
    INCLUDE_DIRECTORIES "${MicrosoftGSL_include_directory}/"
)