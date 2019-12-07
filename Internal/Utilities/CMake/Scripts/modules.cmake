function (target_header _target _module_name _header)

    set (_header_cmi "${CMAKE_BINARY_DIR}/cmi/${_module_name}.pcm")

    add_custom_command (
        OUTPUT "${_header_cmi}"
        COMMAND 
            "${CMAKE_CXX_COMPILER}" "${CMAKE_CXX_FLAGS}" "-std=c++2a" "-fretain-comments-from-system-headers"
            "-fmodule-name=${_module_name}"
            "-x" "c++-header" "${_header}"
            "-Xclang" "-emit-header-module" "-o" "${_header_cmi}"
        MAIN_DEPENDENCY "${_header}"
    )

    add_custom_target (
        "build_header_unit_${_module_name}"
        DEPENDS "${_header_cmi}"
    )

    target_sources (
        ${_target} 
        PRIVATE
            "${_header}"  
    )

    add_dependencies (${_target} "build_header_unit_${_module_name}")

    get_target_property (_header_cmi_list ${_target} HEADER_CMI_LIST)
    if (NOT _header_cmi_list)
        set (_header_cmi_list)
    endif ()
    list (APPEND _header_cmi_list "${_header_cmi}")
    set_target_properties (${_target} PROPERTIES HEADER_CMI_LIST "${_header_cmi_list}")

endfunction ()

function (create_module_options _output_command _module_dependencies _prebuilt_module_path)

    set (_command)
    list (APPEND _command 
        "-fimplicit-modules" "-fimplicit-module-maps" "-fprebuilt-module-path=${_prebuilt_module_path}"
        "-fretain-comments-from-system-headers"
    )

    foreach (_module_dependency IN LISTS _module_dependencies)

        if (TARGET ${_module_dependency})
            get_target_property (_header_cmi_list ${_module_dependency} HEADER_CMI_LIST)

            foreach (_header_cmi IN LISTS _header_cmi_list)
                list(APPEND _command "-fmodule-file=${_header_cmi}")
            endforeach ()
        else ()
            list(APPEND _command "-fmodule-file=${_prebuilt_module_path}/${_module_dependency}.pcm")
        endif ()

    endforeach ()

    set (${_output_command} ${_command} PARENT_SCOPE)

endfunction ()

function (target_module _target _module_name)

    set (options)
    set (oneValueArgs INTERFACE IMPLEMENTATION)
    set (multiValueArgs INTERFACE_DEPENDS IMPLEMENTATION_DEPENDS)
    cmake_parse_arguments (ARG "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    set (_prebuilt_module_path "${CMAKE_BINARY_DIR}/cmi")
    set (_module_cmi "${_prebuilt_module_path}/${_module_name}.pcm")
    set (_module_interface_unit "${CMAKE_CURRENT_SOURCE_DIR}/${ARG_INTERFACE}")
    set (_module_interface_unit_object "${CMAKE_CURRENT_BINARY_DIR}/objects/${ARG_INTERFACE}.o")
    set (_module_interface_unit_dependencies "${ARG_INTERFACE_DEPENDS}")
    if (ARG_IMPLEMENTATION)
        set (_module_implementation_unit "${CMAKE_CURRENT_SOURCE_DIR}/${ARG_IMPLEMENTATION}")
        set (_module_implementation_unit_object "${CMAKE_CURRENT_BINARY_DIR}/objects/${ARG_IMPLEMENTATION}.o")
        set (_module_implementation_unit_dependencies "${ARG_IMPLEMENTATION_DEPENDS}")
    endif ()


    create_module_options (_module_interface_options "${_module_interface_unit_dependencies}" "${_prebuilt_module_path}")

    set(_module_interfacec_unit_cmi_dependencies)
    foreach (_module_dependency IN LISTS _module_interface_unit_dependencies)
        if (NOT TARGET ${_module_dependency})
            list (APPEND _module_interfacec_unit_cmi_dependencies "${_prebuilt_module_path}/${_module_dependency}.pcm")
        else ()
            list (APPEND _module_interfacec_unit_cmi_dependencies ${_module_dependency})
        endif ()
    endforeach ()
    

    add_custom_command (
        OUTPUT "${_module_cmi}"
        COMMAND 
            "${CMAKE_CXX_COMPILER}" "${CMAKE_CXX_FLAGS}" "-std=c++2a" "-fretain-comments-from-system-headers"
            ${_module_interface_options}
            "-x" "c++" "${_module_interface_unit}" "-c"
            "-Xclang" "-emit-module-interface" "-o" "${_module_cmi}"
        DEPENDS "${_module_interface_unit}" "${_module_interfacec_unit_cmi_dependencies}"
    )

    add_custom_target (
        build_${_module_name}_cmi
        DEPENDS "${_module_cmi}"
    )

    add_dependencies (${_target} build_${_module_name}_cmi)


    foreach (_dependency IN LISTS _module_interface_unit_dependencies _module_implementation_unit_dependencies)
        if (TARGET ${_dependency})
            target_link_libraries (${_target} PRIVATE ${_dependency})
        endif ()
    endforeach ()

    target_sources (${_target} PRIVATE "${_module_interface_unit}")
    set_source_files_properties("${_module_interface_unit}" PROPERTIES COMPILE_OPTIONS "${_module_interface_options}")


    if (_module_implementation_unit)

        target_sources (${_target} PRIVATE "${_module_implementation_unit}")

        list (APPEND _module_implementation_unit_dependencies "${_module_name}")
        create_module_options (_module_implementation_options "${_module_implementation_unit_dependencies}" "${_prebuilt_module_path}")
        set_source_files_properties("${_module_implementation_unit}" PROPERTIES COMPILE_OPTIONS "${_module_implementation_options}")

    endif ()

endfunction ()

function (add_module_compile_options _target)

    target_compile_options (${_target} PRIVATE
        "-std=c++2a"  
        "-fimplicit-modules" 
        "-fimplicit-module-maps" 
        "-fprebuilt-module-path=${CMAKE_BINARY_DIR}/cmi"
        "-fretain-comments-from-system-headers"
    )

endfunction ()

function (add_module_dependency _target _module_names)

    foreach (_module_name IN LISTS _module_names)
        
        set (_module_cmi "${CMAKE_BINARY_DIR}/cmi/${_module_name}.pcm")

        target_compile_options (${_target} PRIVATE 
            "-fmodule-file=${_module_cmi}"
        )

    endforeach ()

endfunction ()