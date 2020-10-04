project (MaiaECS VERSION 0.0.1)

add_library (MaiaECS)
add_library (Maia::ECS ALIAS MaiaECS)

set_target_properties(MaiaECS 
	PROPERTIES 
		LINKER_LANGUAGE CXX
)

target_include_directories (MaiaECS 
	PUBLIC
		$<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}>
)

find_package (Eigen3 3.3.7 CONFIG REQUIRED)
target_link_libraries (MaiaECS PUBLIC Eigen3::Eigen)

target_module (
    MaiaECS
    PUBLIC
    "maia.ecs.component"
    INTERFACE "Component.cxx"
    INTERFACE_DEPENDS "Standard_library"
    IMPLEMENTATION "Component.cpp"
    IMPLEMENTATION_DEPENDS "Standard_library"
)

target_module (
    MaiaECS
    PUBLIC
    "maia.ecs.shared_component"
    INTERFACE "Shared_component.cxx"
    INTERFACE_DEPENDS "Standard_library"
    IMPLEMENTATION "Shared_component.cpp"
    IMPLEMENTATION_DEPENDS "Standard_library"
)

target_module (
    MaiaECS
    PUBLIC
    "maia.ecs.archetype"
    INTERFACE "Archetype.cxx"
    INTERFACE_DEPENDS "Standard_library"
        "maia.ecs.component"
        "maia.ecs.shared_component"
)

target_module (
    MaiaECS
    PUBLIC
    "maia.ecs.component_group_mask"
    INTERFACE "Component_group_mask.cxx"
	INTERFACE_DEPENDS "Standard_library"
		"maia.ecs.component"
    IMPLEMENTATION "Component_group_mask.cpp"
    IMPLEMENTATION_DEPENDS "Standard_library"
        "maia.ecs.component"
)

target_module (
    MaiaECS
    PUBLIC
    "maia.ecs.components_chunk"
    INTERFACE "Components_chunk.cxx"
	INTERFACE_DEPENDS "Standard_library"
		"maia.ecs.component"
    IMPLEMENTATION "Components_chunk.cpp"
    IMPLEMENTATION_DEPENDS "Standard_library"
)

target_module (
    MaiaECS
    PUBLIC
    "maia.ecs.entity"
    INTERFACE "Entity.cxx"
	INTERFACE_DEPENDS "Standard_library"
    IMPLEMENTATION "Entity.cpp"
    IMPLEMENTATION_DEPENDS "Standard_library"
)

target_module (
    MaiaECS
    PUBLIC
    "maia.ecs.entity_type"
    INTERFACE "Entity_type.cxx"
	INTERFACE_DEPENDS "Standard_library"
    IMPLEMENTATION "Entity_type.cpp"
    IMPLEMENTATION_DEPENDS "Standard_library"
)

target_module (
    MaiaECS
    PUBLIC
    "maia.ecs.component_group"
    INTERFACE "Component_group.cxx"
	INTERFACE_DEPENDS "Standard_library"
		"maia.ecs.component"
		"maia.ecs.components_chunk"
		"maia.ecs.entity"
    IMPLEMENTATION "Component_group.cpp"
	IMPLEMENTATION_DEPENDS "Standard_library"
		"maia.ecs.component"
		"maia.ecs.components_chunk"
		"maia.ecs.entity"
)

target_module (
    MaiaECS
    PUBLIC
    "maia.ecs.components_view"
    INTERFACE "Components_view.cxx"
    INTERFACE_DEPENDS "Standard_library"
        "maia.ecs.component"
        "maia.ecs.components_chunk"
        "maia.ecs.shared_component"
)

target_module (
    MaiaECS
    PUBLIC
    "maia.ecs.entity_manager"
    INTERFACE "Entity_manager.cxx"
	INTERFACE_DEPENDS "Standard_library"
		"maia.ecs.component"
		"maia.ecs.component_group"
		"maia.ecs.component_group_mask"
		"maia.ecs.components_chunk"
		"maia.ecs.entity"
		"maia.ecs.entity_type"
    IMPLEMENTATION "Entity_manager.cpp"
	IMPLEMENTATION_DEPENDS "Standard_library"
		"maia.ecs.component"
		"maia.ecs.component_group"
		"maia.ecs.component_group_mask"
		"maia.ecs.components_chunk"
		"maia.ecs.entity"
		"maia.ecs.entity_type"
)

target_module (
    MaiaECS
    PUBLIC
    "maia.ecs.world"
    INTERFACE "World.cxx"
    INTERFACE_DEPENDS "Standard_library"
        "maia.ecs.archetype"
        "maia.ecs.component"
        "maia.ecs.components_chunk"
        "maia.ecs.components_view"
        "maia.ecs.entity"
        "maia.ecs.shared_component"
)

add_subdirectory("Components")
add_subdirectory("Systems")