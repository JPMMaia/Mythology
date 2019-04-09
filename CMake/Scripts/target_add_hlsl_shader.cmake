function(target_add_hlsl_shader target_name group input_filename output_filename entry_point_name target_profile additional_options)

	find_program(FXC "fxc.exe")

	add_custom_command(
		COMMAND ${FXC} ARGS /E "${entry_point_name}" /T "${target_profile}" /Fo "${output_filename}" "${additional_options}" "${input_filename}"
		MAIN_DEPENDENCY "${input_filename}"
		OUTPUT "${output_filename}"
		USES_TERMINAL
	)

	target_sources(${target_name} PRIVATE 
		"${input_filename}"
		"${output_filename}"
	)

	source_group("Resource Files\\${group}" FILES "${input_filename}")
	source_group("Generated Files\\${group}" FILES "${output_filename}")

	set_source_files_properties(
		"${output_filename}"
			PROPERTIES 
				VS_COPY_TO_OUT_DIR PreserveNewest
				VS_DEPLOYMENT_CONTENT 1
				VS_DEPLOYMENT_LOCATION "${group}"
	)

endfunction()
