add_library(MaiaECSSystems)
add_library(Maia::ECS::Systems ALIAS MaiaECSSystems)

set_target_properties(MaiaECSSystems 
	PROPERTIES 
		LINKER_LANGUAGE CXX
)

target_link_libraries(MaiaECSSystems PUBLIC MaiaECS)

target_module(
    MaiaECSSystems
    PUBLIC
    "maia.ecs.systems.transform_system"
    INTERFACE "Transform_system.cppm"
)
