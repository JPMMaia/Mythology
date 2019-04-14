function (target_resources target)


    if (CMAKE_RUNTIME_OUTPUT_DIRECTORY)
        set (RESOURCES_OUTPUT_DIRECTORY "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${CMAKE_CFG_INTDIR}/Resources")
    else ()
        set (RESOURCES_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/Resources/${CMAKE_CFG_INTDIR}")
    endif ()


    set (options)
    set (oneValueArgs)
    set (multiValueArgs PUBLIC INTERFACE PRIVATE)
    cmake_parse_arguments (TARGET_RESOURCES "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    if (TARGET_RESOURCES_PUBLIC)
        target_sources (${target} PUBLIC ${TARGET_RESOURCES_PUBLIC})
    endif ()

    if (TARGET_RESOURCES_INTERFACE)
        target_sources (${target} INTERFACE ${TARGET_RESOURCES_INTERFACE})
    endif ()

    if (TARGET_RESOURCES_PRIVATE)
        target_sources (${target} PRIVATE ${TARGET_RESOURCES_PRIVATE})
    endif ()


    foreach (resource IN LISTS TARGET_RESOURCES_PUBLIC TARGET_RESOURCES_INTERFACE TARGET_RESOURCES_PRIVATE)

        add_custom_command (
            OUTPUT "${RESOURCES_OUTPUT_DIRECTORY}/${resource}"
            COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_CURRENT_SOURCE_DIR}/${resource}" "${RESOURCES_OUTPUT_DIRECTORY}/${resource}"
            MAIN_DEPENDENCY "${CMAKE_CURRENT_SOURCE_DIR}/${resource}"
        )

    endforeach ()

    source_group ("Source" FILES ${TARGET_RESOURCES_PUBLIC} ${TARGET_RESOURCES_INTERFACE} ${TARGET_RESOURCES_PRIVATE})

endfunction ()