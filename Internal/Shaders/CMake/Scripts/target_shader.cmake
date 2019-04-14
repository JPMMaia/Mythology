function (target_shader copy_target compile_target)


    if (CMAKE_RUNTIME_OUTPUT_DIRECTORY)
        set (SHADER_BINARY_OUTPUT_DIRECTORY "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${CMAKE_CFG_INTDIR}/Shaders")
    else ()
        set (SHADER_BINARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/Binaries/${CMAKE_CFG_INTDIR}/Shaders")
    endif ()


    set (state "None")
    set (input "")
    set (type "")
    set (intermediate_lists)

    
    foreach (argument IN LISTS ARGV)

        if (${argument} STREQUAL "INPUT")
            set (state "Input")

        elseif (${argument} STREQUAL "TYPE")
            set (state "Type")
        
        elseif (${argument} STREQUAL "INTERMEDIATE")
            set (state "Intermediate")

        elseif (${argument} STREQUAL "MODEL")
            set (state "Model")

        elseif (${argument} STREQUAL "ENTRYPOINT")
            set (state "Entrypoint")

        elseif (${argument} STREQUAL "FLAGS")
            set (state "Flags")
        
        elseif (state STREQUAL "Input")

            set (input ${argument})
            set (state "None")

        elseif (state STREQUAL "Type")

            set (type ${argument})
            set (state "None")

        elseif (state STREQUAL "Intermediate")

            list (APPEND intermediate_lists "${argument}")
            set (state "None")
   
        elseif (state STREQUAL "Model")

            list (GET intermediate_lists -1 list_name)
            set ("${list_name}_model" ${argument})
            set (state "None")

        elseif (state STREQUAL "Entrypoint")

            list (GET intermediate_lists -1 list_name)
            set ("${list_name}_entrypoint" ${argument})
            set (state "None")

        elseif (state STREQUAL "Flags")

            list (GET intermediate_lists -1 list_name)
            list (APPEND "${list_name}_flags" ${argument})

        endif ()

    endforeach ()


    set (intermediate_outputs ${intermediate_lists})
    list (TRANSFORM intermediate_outputs PREPEND "${CMAKE_CURRENT_BINARY_DIR}/Intermediate/${CMAKE_CFG_INTDIR}/")

    set (commands_list)
    foreach (intermediate_output IN LISTS intermediate_outputs)
        list (APPEND commands_list COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_CURRENT_SOURCE_DIR}/${input}" "${intermediate_output}")
    endforeach ()


    add_custom_command (
        OUTPUT ${intermediate_outputs}
        ${commands_list}
        MAIN_DEPENDENCY ${input}
    )


    set (shader_binary_outputs ${intermediate_lists})
    list (TRANSFORM shader_binary_outputs PREPEND "${SHADER_BINARY_OUTPUT_DIRECTORY}/")
    list (TRANSFORM shader_binary_outputs REPLACE "\\.[^.]*$" ".cso")

    target_sources (${copy_target}
        PRIVATE
            ${input}
    )

    target_sources (${compile_target}
        PRIVATE
            ${intermediate_outputs}
    )

    list (LENGTH intermediate_outputs num_shaders_to_generate)
    MATH (EXPR last_index "${num_shaders_to_generate} - 1")
    foreach (index RANGE 0 ${last_index})
    
        list (GET intermediate_lists ${index} list_name)
        list (GET intermediate_outputs ${index} intermediate_output)
        list (GET shader_binary_outputs ${index} shader_binary_output)

        set_source_files_properties ("${intermediate_output}" PROPERTIES
            VS_SHADER_DISABLE_OPTIMIZATIONS $<IF:$<CONFIG:Debug>,true,false>
            VS_SHADER_ENABLE_DEBUG $<OR:$<CONFIG:Debug>,$<CONFIG:RelWithDebInfo>>
            VS_SHADER_OBJECT_FILE_NAME "${shader_binary_output}"
            VS_SHADER_TYPE "${type}"
        )

        if (${list_name}_model)
            set_source_files_properties ("${intermediate_output}" PROPERTIES
                VS_SHADER_MODEL ${${list_name}_model}
            )
        else ()
           set_source_files_properties ("${intermediate_output}" PROPERTIES
                VS_SHADER_MODEL 6.0
            )
        endif ()

        if (${list_name}_entrypoint)
            set_source_files_properties ("${intermediate_output}" PROPERTIES
                VS_SHADER_ENTRYPOINT ${${list_name}_entrypoint}
            )
        endif ()

        if (${list_name}_flags)

            list(JOIN ${list_name}_flags " " flags)
            set_source_files_properties ("${intermediate_output}" PROPERTIES
                VS_SHADER_FLAGS ${flags}
            )
        endif ()

    endforeach ()

    source_group ("Source" FILES ${input} ${intermediate_outputs})

endfunction ()