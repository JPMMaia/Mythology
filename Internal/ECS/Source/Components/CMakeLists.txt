project (MaiaECSComponents VERSION 0.0.1)

add_library (MaiaECSComponents)
add_library (Maia::ECS::Components ALIAS MaiaECSComponents)

set_target_properties(MaiaECSComponents 
	PROPERTIES 
		LINKER_LANGUAGE CXX
)

target_compile_features (MaiaECSComponents 
	PRIVATE 
		cxx_std_20
)

target_module (
    MaiaECSComponents
    PUBLIC
    "maia.ecs.components.local_position"
    INTERFACE "Local_position.cxx"
)

target_module (
    MaiaECSComponents
    PUBLIC
    "maia.ecs.components.local_rotation"
    INTERFACE "Local_rotation.cxx"
)
