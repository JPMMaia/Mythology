add_library (MaiaECS)
add_library (Maia::ECS ALIAS MaiaECS)

set_target_properties(MaiaECS 
	PROPERTIES 
		LINKER_LANGUAGE CXX
)

target_module(
    MaiaECS
    PUBLIC
    "maia.ecs.tuple_helpers"
    INTERFACE "Tuple_helpers.cxx"
)

target_module(
    MaiaECS
    PUBLIC
    "maia.ecs.component_groups"
    INTERFACE "Component_groups.cxx"
    INTERFACE_DEPENDS
        "maia.ecs.tuple_helpers"
)

target_module(
    MaiaECS
    PUBLIC
    "maia.ecs.entity"
    INTERFACE "Entity.cxx"
    IMPLEMENTATION "Entity.cpp"
)

target_module(
    MaiaECS
    PUBLIC
    "maia.ecs.entity_manager"
    INTERFACE "Entity_manager.cxx"
    INTERFACE_DEPENDS
        "maia.ecs.component_groups"
        "maia.ecs.entity"
        "maia.ecs.tuple_helpers"
)
