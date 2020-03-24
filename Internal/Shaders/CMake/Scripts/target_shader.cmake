function (target_shader _target _dxc)

    set (options)
    set (oneValueArgs INPUT OUTPUT TEMP_DIRECTORY OUTPUT_DIRECTORY)
    set (multiValueArgs DEFINES FLAGS)
    cmake_parse_arguments (ARG "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    set (_input "${CMAKE_CURRENT_LIST_DIR}/${ARG_INPUT}")
    set (_output "${ARG_OUTPUT}")
    set (_defines ${ARG_DEFINES})
    set (_flags ${ARG_FLAGS})

    if (ARG_TEMP_DIRECTORY)
        set (_temp_directory "${ARG_TEMP_DIRECTORY}")
    else ()
        set (_temp_directory "${CMAKE_CURRENT_BINARY_DIR}/tmp/shaders")
    endif ()

    if (ARG_OUTPUT_DIRECTORY)
        set (_output_directory "${ARG_OUTPUT_DIRECTORY}")
    else ()
        set (_output_directory "${CMAKE_CURRENT_BINARY_DIR}/shaders")
    endif ()

    
    set (_intermediate_file_path "${_temp_directory}/${_output}.hlsl")

    add_custom_command (
        OUTPUT "${_intermediate_file_path}"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different "${_input}" "${_intermediate_file_path}"
        DEPENDS "${_input}"
        VERBATIM
    )

    add_custom_target (
        "${_output}_copy"
        DEPENDS "${_intermediate_file_path}"
        VERBATIM
    )


    set (_output_file_path "${_output_directory}/${_output}")

    add_custom_command (
        OUTPUT "${_output_file_path}"
        COMMAND ${_dxc} ${_flags} ${_defines} "${_intermediate_file_path}" -Fo "${_output_file_path}"
        DEPENDS "${_intermediate_file_path}"
        VERBATIM
    )

    add_custom_target (
        "${_output}_compile"
        DEPENDS "${_output_file_path}"
        VERBATIM
    )

    add_dependencies("${_target}" "${_output}_compile")
    target_sources ("${_target}" PRIVATE "${_input}")

endfunction ()