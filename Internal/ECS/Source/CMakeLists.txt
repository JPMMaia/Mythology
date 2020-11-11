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
    "maia.ecs.entity"
    INTERFACE "Entity.cxx"
	INTERFACE_DEPENDS "Standard_library"
    IMPLEMENTATION "Entity.cpp"
    IMPLEMENTATION_DEPENDS "Standard_library"
)

target_module (
    MaiaECS
    PUBLIC
    "maia.ecs.component_chunk"
    INTERFACE "Component_chunk.cxx"
    INTERFACE_DEPENDS "Standard_library"
        "maia.ecs.component"
        "maia.ecs.entity"
)

target_module (
    MaiaECS
    PUBLIC
    "maia.ecs.components_view"
    INTERFACE "Components_view.cxx"
    INTERFACE_DEPENDS "Standard_library"
        "maia.ecs.component"
        "maia.ecs.entity"
        "maia.ecs.shared_component"
)

target_module (
    MaiaECS
    PUBLIC
    "maia.ecs.component_chunk_group"
    INTERFACE "Component_chunk_group.cxx"
    INTERFACE_DEPENDS "Standard_library"
        "maia.ecs.component"
        "maia.ecs.entity"
        "maia.ecs.shared_component"
    IMPLEMENTATION "Component_chunk_group.cpp"
    IMPLEMENTATION_DEPENDS "Standard_library"
        "maia.ecs.component"
        "maia.ecs.entity"
        "maia.ecs.shared_component"
)

target_module (
    MaiaECS
    PUBLIC
    "maia.ecs.entity_manager"
    INTERFACE "Entity_manager.cxx"
    INTERFACE_DEPENDS "Standard_library"
        "maia.ecs.archetype"
        "maia.ecs.component"
        "maia.ecs.component_chunk"
        "maia.ecs.components_view"
        "maia.ecs.component_chunk_group"
        "maia.ecs.entity"
        "maia.ecs.shared_component"
)

target_module (
    MaiaECS
    PUBLIC
    "maia.ecs.world"
    INTERFACE "World.cxx"
    INTERFACE_DEPENDS "Standard_library"
        "maia.ecs.archetype"
        "maia.ecs.component"
        "maia.ecs.component_chunk"
        "maia.ecs.component_chunk_group"
        "maia.ecs.components_view"
        "maia.ecs.entity"
        "maia.ecs.entity_manager"
        "maia.ecs.shared_component"
)

add_subdirectory("Components")
add_subdirectory("Systems")