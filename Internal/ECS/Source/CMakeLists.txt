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
    "maia.ecs.entity"
    INTERFACE "Entity.cxx"
	INTERFACE_DEPENDS "Standard_library"
    IMPLEMENTATION "Entity.cpp"
    IMPLEMENTATION_DEPENDS "Standard_library"
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
    "maia.ecs.entity_manager"
    INTERFACE "Entity_manager.cxx"
    INTERFACE_DEPENDS "Standard_library"
        "maia.ecs.archetype"
        "maia.ecs.component"
        "maia.ecs.components_view"
        "maia.ecs.entity"
        "maia.ecs.shared_component"
)

add_subdirectory("Components")
add_subdirectory("Systems")