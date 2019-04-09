project (MaiaGameEngine VERSION 0.0.1)

add_library (MaiaGameEngine)
add_library (Maia::GameEngine ALIAS MaiaGameEngine)

set_target_properties(MaiaGameEngine 
	PROPERTIES 
		LINKER_LANGUAGE CXX
)

target_compile_features (MaiaGameEngine 
	PRIVATE 
		cxx_std_17
)

target_include_directories (MaiaGameEngine 
	PUBLIC
		$<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}>
		$<INSTALL_INTERFACE:include/Maia/GameEngine>
)

find_package (Eigen3 3.3 NO_MODULE REQUIRED)
target_link_libraries (MaiaGameEngine PUBLIC Eigen3::Eigen)

find_package (GSL REQUIRED)
target_link_libraries (MaiaGameEngine PUBLIC GSL::GSL)

target_sources (MaiaGameEngine 
	PRIVATE
		"Maia/GameEngine/Component.hpp"
		"Maia/GameEngine/Component.cpp"
		"Maia/GameEngine/Component_group.hpp"
		"Maia/GameEngine/Component_group.cpp"
		"Maia/GameEngine/Component_group_mask.hpp"
		"Maia/GameEngine/Component_group_mask.cpp"
		"Maia/GameEngine/Components_chunk.hpp"
		"Maia/GameEngine/Components_chunk.cpp"
		
		"Maia/GameEngine/Entity.hpp"
		"Maia/GameEngine/Entity.cpp"
		"Maia/GameEngine/Entity_hash.hpp"
		"Maia/GameEngine/Entity_hash.cpp"
		"Maia/GameEngine/Entity_manager.hpp"
		"Maia/GameEngine/Entity_manager.cpp"
		"Maia/GameEngine/Entity_type.hpp"
		"Maia/GameEngine/Entity_type.cpp"
		
		"Maia/GameEngine/Components/Local_position.hpp"
		"Maia/GameEngine/Components/Local_position.cpp"
		"Maia/GameEngine/Components/Local_rotation.hpp"
		"Maia/GameEngine/Components/Local_rotation.cpp"

		"Maia/GameEngine/Systems/Transform_system.hpp"
		"Maia/GameEngine/Systems/Transform_system.cpp"
)

install (TARGETS MaiaGameEngine EXPORT MaiaGameEngineTargets
	LIBRARY DESTINATION "lib"
	ARCHIVE DESTINATION "lib"
	RUNTIME DESTINATION "bin"
	INCLUDES DESTINATION "include"
)
install (DIRECTORY "${CMAKE_CURRENT_LIST_DIR}/Maia"
		DESTINATION "include"
        FILES_MATCHING PATTERN "*.hpp"
)
install (EXPORT MaiaGameEngineTargets
	FILE "MaiaGameEngineTargets.cmake"
	NAMESPACE Maia::
	DESTINATION "lib/cmake/Maia/GameEngine"
)

include (CMakePackageConfigHelpers)
write_basic_package_version_file ("${CMAKE_CURRENT_BINARY_DIR}/MaiaGameEngineConfigVersion.cmake"
	VERSION ${MaiaGameEngine_VERSION}
	COMPATIBILITY SameMajorVersion
)
install (FILES "MaiaGameEngineConfig.cmake" "${CMAKE_CURRENT_BINARY_DIR}/MaiaGameEngineConfigVersion.cmake"
	DESTINATION "lib/cmake/Maia/GameEngine"
)
