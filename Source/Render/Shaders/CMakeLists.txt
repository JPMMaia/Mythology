project(Shaders)

add_library(Shaders MODULE)
add_library(Mythology::Shaders ALIAS Shaders)

set_target_properties(Shaders PROPERTIES LINKER_LANGUAGE CXX)

#[[function(target_shader target_name input_filename entry_point target_profile additional_flags)

	find_program(FXC "fxc.exe")

	add_custom_command(
		COMMAND ${FXC} ARGS /E "${entry_point}" /T "${target_profile}" "${additional_options}" "${input_filename}"
		OUTPUT "${input_filename}.csv"
		MAIN_DEPENDENCY "${input_filename}"
		USES_TERMINAL
	)

	target_sources(${target_name} PRIVATE ${input_filename})

    set_source_files_properties(
        ${input_filename}
            PROPERTIES
				VS_COPY_TO_OUT_DIR PreserveNewest
    )

endfunction()
#]]
#arget_shader(Shaders "${CMAKE_CURRENT_LIST_DIR}/Color_vertex_shader.hlsl" "main" "vs_5_1" "/all_resources_bound")
#target_shader(Shaders "${CMAKE_CURRENT_LIST_DIR}/Color_pixel_shader.hlsl" "main" "ps_5_1" "/all_resources_bound")

set(
	SHADER_FILES 
		"Color_vertex_shader.hlsl" "Color_pixel_shader.hlsl"
)

target_sources(Shaders PRIVATE "Dummy.cpp")

add_custom_command(
	TARGET Shaders PRE_BUILD
    COMMAND cmake -E copy_if_different ${SHADER_FILES} "${CMAKE_BINARY_DIR}/Output/${CMAKE_CFG_INTDIR}"
	WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
	USES_TERMINAL
)