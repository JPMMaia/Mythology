project (MaiaECSSystems VERSION 0.0.1)

add_library (MaiaECSSystems)
add_library (Maia::ECS::Systems ALIAS MaiaECSSystems)

set_target_properties(MaiaECSSystems 
	PROPERTIES 
		LINKER_LANGUAGE CXX
)

target_compile_features (MaiaECSSystems 
	PRIVATE 
		cxx_std_20
)

find_package (Eigen3 3.3.7 CONFIG REQUIRED)
target_link_libraries (MaiaECSSystems PUBLIC Eigen3::Eigen)

target_link_libraries (MaiaECSSystems PUBLIC Maia::ECS)
target_link_libraries (MaiaECSSystems PUBLIC Maia::ECS::Components)

target_module (
    MaiaECSSystems
    "maia.ecs.systems.transform_system"
    INTERFACE "Transform_system.cxx"
    INTERFACE_DEPENDS "Eigen3_module" "Standard_library"
    IMPLEMENTATION "Transform_system.cpp"
    IMPLEMENTATION_DEPENDS "Eigen3_module" "Standard_library"
)
