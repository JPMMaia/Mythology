if (MSVC)

    get_property(_is_multi_config GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)

    if (_is_multi_config)
        set(_output_ifc_dir "${CMAKE_BINARY_DIR}/${CMAKE_CFG_INTDIR}/ifc")
        file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/Debug/ifc")
        file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/Release/ifc")
    else ()
        set(_output_ifc_dir "${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE}/ifc")
        file(MAKE_DIRECTORY "${_output_ifc_dir}")
    endif ()
    

    find_program(
        msvc_compiler
        "${CMAKE_CXX_COMPILER}"
        REQUIRED
    )

    function (get_dependencies_ifc_outputs _output_variable _dependencies)

        set(_ifc_outputs)

        foreach (_dependency IN LISTS _dependencies)

            if (NOT TARGET "${_dependency}")
                list(APPEND _ifc_outputs "${_output_ifc_dir}/${_dependency}.ifc")
            endif ()

        endforeach ()

        set(${_output_variable} ${_ifc_outputs} PARENT_SCOPE)

    endfunction ()

    function (create_dependencies_commands _output_variable _dependencies)

        set(_commands)

        foreach (_dependency IN LISTS _dependencies)

            if (NOT TARGET "${_dependency}")

                set(_ifc_output "${_output_ifc_dir}/${_dependency}.ifc")

                get_source_file_property(
                    _is_header_unit
                    "${_ifc_output}"
                    HEADER_UNIT
                )

                if (${_is_header_unit})

                    get_source_file_property(
                        _header_source
                        "${_ifc_output}"
                        HEADER_SOURCE
                    )

                    list(APPEND _commands "/headerUnit" "${_header_source}=${_ifc_output}")
                else ()
                    list(APPEND _commands "/reference" "${_dependency}=${_ifc_output}")
                endif ()

            endif ()

        endforeach ()

        set(${_output_variable} ${_commands} PARENT_SCOPE)

    endfunction ()

    function (add_build_interface_target _target _visibility _module_name _interface _interface_dependencies)

        get_filename_component(_interface_filename "${_interface}" NAME)
        set(_interface_ifc_output "${_output_ifc_dir}/${_module_name}.ifc")

        create_dependencies_commands(_interface_dependencies_commands "${_interface_dependencies}")
        message(DEBUG "_interface_dependencies_commands is ${_interface_dependencies_commands}")

        get_dependencies_ifc_outputs(_interface_ifc_dependencies "${_interface_dependencies}")
        message(DEBUG "_interface_ifc_dependencies is ${_interface_ifc_dependencies}")

        target_sources("${_target}" PRIVATE "${_interface}")

        set_source_files_properties(
            "${_interface}"
                PROPERTIES
                    COMPILE_OPTIONS "${_interface_dependencies_commands};/interface;/ifcOutput;${_interface_ifc_output}"
                    OBJECT_DEPENDS "${_interface_ifc_dependencies}"
                    OBJECT_OUTPUTS "${_interface_ifc_output}"
        )

        foreach (_dependency IN LISTS _interface_dependencies)

            if (TARGET "${_dependency}")

                target_link_libraries("${_target}" PRIVATE "${_dependency}")

            endif ()

        endforeach ()

        if (_visibility STREQUAL "PUBLIC")
            target_compile_options(
                "${_target}"
                INTERFACE
                "SHELL:/reference \"${_module_name}=${_interface_ifc_output}\""
            )
        endif ()

    endfunction ()

    function (add_build_implementation_target _target _module_name _implementation _implementation_dependencies)

        create_dependencies_commands(_implementation_dependencies_commands "${_implementation_dependencies}")
        message(DEBUG "_implementation_dependencies_commands is ${_implementation_dependencies_commands}")

        get_dependencies_ifc_outputs(_implementation_ifc_dependencies "${_implementation_dependencies}")
        message(DEBUG "_implementation_ifc_dependencies is ${_implementation_ifc_dependencies}")

        set(_interface_ifc "${_output_ifc_dir}/${_module_name}.ifc")
        message(DEBUG "_interface_ifc is ${_interface_ifc}")

        target_sources("${_target}" PRIVATE "${_implementation}")

        set_source_files_properties(
            "${_implementation}"
                PROPERTIES
                    COMPILE_OPTIONS "/reference;${_module_name}=${_interface_ifc};${_implementation_dependencies_commands}"
                    OBJECT_DEPENDS "${_interface_ifc};${_implementation_ifc_dependencies}"
        )

        foreach (_dependency IN LISTS _implementation_dependencies)

            if (TARGET "${_dependency}")

                target_link_libraries("${_target}" PRIVATE "${_dependency}")

            endif ()

        endforeach ()

    endfunction ()

endif ()

function (target_header _target _visibility _module_name _header)

    set (options)
    set (oneValueArgs)
    set (multiValueArgs COMPILE_OPTIONS)
    cmake_parse_arguments (ARG "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    if (MSVC)

        target_sources(${_target} PRIVATE "${_header}")

        set_source_files_properties(
            "${_header}"
                PROPERTIES
                    COMPILE_OPTIONS "/exportHeader"
                    LANGUAGE "CXX"
        )
#[[
        get_filename_component(_filename "${_header}" NAME)
        set(_ifc_output "${_output_ifc_dir}/${_module_name}.ifc")

        target_sources(${_target} PRIVATE "${_header}")
        
        set_source_files_properties(
            "${_header}"
                PROPERTIES
                    COMPILE_OPTIONS "/exportHeader;/ifcOutput;${_ifc_output}"
                    LANGUAGE "CXX"
                    OBJECT_OUTPUTS "${_ifc_output}"
        )

        set_source_files_properties(
            "${_ifc_output}"
                PROPERTIES
                    GENERATED TRUE
                    HEADER_UNIT TRUE
                    HEADER_SOURCE "${_header}"
        )

        if (_visibility STREQUAL "PUBLIC")
            target_compile_options("${_target}" INTERFACE "SHELL:/headerUnit \"${_header}=${_ifc_output}\"")
        endif ()
        #]]

    else ()

        set (_compile_options ${ARG_COMPILE_OPTIONS})
        set (_header_cmi "${CMAKE_BINARY_DIR}/cmi/${_module_name}.pcm")

        add_custom_command (
            OUTPUT "${_header_cmi}"
            COMMAND 
                "${CMAKE_CXX_COMPILER}" "${CMAKE_CXX_FLAGS}" "-std=c++2a" "-fretain-comments-from-system-headers"
                "-fmodule-name=${_module_name}"
                "-x" "c++-header" "${_header}"
                ${_compile_options}
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

    endif ()

endfunction ()

function (get_target_include_directories _target _output_include_directories)
    set (_include_directories)
    get_target_property (_target_dependencies ${_target} LINK_LIBRARIES)
    foreach (_target_dependency IN LISTS _target_dependencies)
        if (TARGET ${_target_dependency})
            get_target_property (_dependency_include_directories ${_target_dependency} INTERFACE_INCLUDE_DIRECTORIES)
            if (_dependency_include_directories)
                list (APPEND _include_directories ${_dependency_include_directories})
            endif ()
        endif ()
    endforeach ()
    set (${_output_include_directories} "${_include_directories}" PARENT_SCOPE)
endfunction ()

function (create_module_options _output_command _module_dependencies _prebuilt_module_path)

    if (MSVC)
    
    else ()

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

    endif ()

endfunction ()

function (target_module _target _visibility _module_name)

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

    if (MSVC)

        target_sources("${_target}" PRIVATE "${_module_interface_unit}" "${_module_implementation_unit}")

        set_source_files_properties("${_module_interface_unit}"
                PROPERTIES
                    COMPILE_OPTIONS "/interface"
        )
#[[
        add_build_interface_target(
            "${_target}"
            "${_visibility}"
            "${_module_name}"
            "${_module_interface_unit}"
            "${_module_interface_unit_dependencies}"
        )

        if (ARG_IMPLEMENTATION)
            add_build_implementation_target(
                "${_target}"
                "${_module_name}"
                "${_module_implementation_unit}"
                "${_module_implementation_unit_dependencies}"
            )
        endif ()
#]]
    else ()

        create_module_options (_module_interface_options "${_module_interface_unit_dependencies}" "${_prebuilt_module_path}")

        set(_module_interfacec_unit_cmi_dependencies)
        foreach (_module_dependency IN LISTS _module_interface_unit_dependencies)
            if (NOT TARGET ${_module_dependency})
                list (APPEND _module_interfacec_unit_cmi_dependencies "${_prebuilt_module_path}/${_module_dependency}.pcm")
            else ()
                list (APPEND _module_interfacec_unit_cmi_dependencies ${_module_dependency})
            endif ()
        endforeach ()
        
        get_target_include_directories (${_target} _include_directories)
        list (TRANSFORM _include_directories PREPEND "-I")

        add_custom_command (
            OUTPUT "${_module_cmi}"
            COMMAND 
                "${CMAKE_CXX_COMPILER}" "${CMAKE_CXX_FLAGS}" "-std=c++2a" "-fretain-comments-from-system-headers"
                ${_module_interface_options}
                "-x" "c++" "${_module_interface_unit}" "-c"
                "-Xclang" "-emit-module-interface" "-o" "${_module_cmi}"
                ${_include_directories}
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

    endif ()


endfunction ()

function (add_module_compile_options _target)

    if (MSVC)
        
        target_compile_options (${_target} PRIVATE
            "/std:c++latest"
            "/Zc:preprocessor"
        )

    else ()

        target_compile_options (${_target} PRIVATE
            "-std=c++2a"  
            "-fimplicit-modules" 
            "-fimplicit-module-maps" 
            "-fprebuilt-module-path=${CMAKE_BINARY_DIR}/cmi"
            "-fretain-comments-from-system-headers"
        )

    endif ()

endfunction ()

function (add_module_dependency _target _module_names)

    if (MSVC)

    else ()

        foreach (_module_name IN LISTS _module_names)
            
            set (_module_cmi "${CMAKE_BINARY_DIR}/cmi/${_module_name}.pcm")

            target_compile_options (${_target} PRIVATE 
                "-fmodule-file=${_module_cmi}"
            )

        endforeach ()

    endif ()

endfunction ()