function (target_shader_header copy_target compile_target)

    set (options)
    set (oneValueArgs)
    set (multiValueArgs INPUTS)
    cmake_parse_arguments (TARGET_SHADER_HEADER "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    list (LENGTH TARGET_SHADER_HEADER_INPUTS inputs_length)
    set (inputs ${TARGET_SHADER_HEADER_INPUTS})

    set (intermediates ${inputs})
    list (TRANSFORM intermediates PREPEND "${CMAKE_CURRENT_BINARY_DIR}/Intermediate/${CMAKE_CFG_INTDIR}/")

    MATH (EXPR last_index "${inputs_length} - 1")
    foreach (index RANGE 0 ${last_index})
    
        list (GET inputs ${index} input)
        list (GET intermediates ${index} intermediate)

        add_custom_command (
            OUTPUT "${intermediate}"
            COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_CURRENT_SOURCE_DIR}/${input}" "${intermediate}"
            MAIN_DEPENDENCY "${CMAKE_CURRENT_SOURCE_DIR}/${input}"
        )

    endforeach ()

    target_sources (${copy_target}
        PRIVATE
            ${inputs}
    )

    target_sources (${compile_target}
        PRIVATE
            ${intermediates}
    )

    source_group (TREE "${CMAKE_CURRENT_SOURCE_DIR}" PREFIX "Source" FILES ${inputs})
    source_group (TREE "${CMAKE_CURRENT_BINARY_DIR}/Intermediate/${CMAKE_CFG_INTDIR}" PREFIX "Source" FILES ${intermediates})

endfunction ()