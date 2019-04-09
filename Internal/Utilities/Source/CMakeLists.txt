project (MaiaUtilities VERSION 0.0.1)

add_library (MaiaUtilities)
add_library (Maia::Utilities ALIAS MaiaUtilities)

set_target_properties (MaiaUtilities PROPERTIES
	LINKER_LANGUAGE "CXX"
)

target_compile_features (MaiaUtilities 
	PUBLIC
		cxx_std_17
)

target_include_directories (MaiaUtilities 
	PUBLIC 
		$<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}>
		$<INSTALL_INTERFACE:include/Maia/Utilities>
)

find_package (Eigen3 3.3 NO_MODULE REQUIRED)
target_link_libraries (MaiaUtilities PUBLIC Eigen3::Eigen)

find_package (GSL REQUIRED)
target_link_libraries (MaiaUtilities PUBLIC GSL::GSL)

find_package (nlohmann_json 3.5.0 REQUIRED)
target_link_libraries (MaiaUtilities PUBLIC nlohmann_json::nlohmann_json)

target_sources(MaiaUtilities 
	PRIVATE

		"Maia/Utilities/Allocators/Buddy_allocator.hpp"
		"Maia/Utilities/Allocators/Forward_allocator.hpp"
		"Maia/Utilities/Allocators/Memory_arena.hpp"
		
		"Maia/Utilities/Containers/Pools/ContiguousMemoryPool.hpp"
		"Maia/Utilities/Containers/Pools/MemoryPool.hpp"
		"Maia/Utilities/Containers/Pools/PointedContiguousMemoryPool.hpp"

		"Maia/Utilities/Containers/Chunks/Memory_chunk.hpp"
		"Maia/Utilities/Containers/Chunks/Memory_chunks.hpp"
		
		"Maia/Utilities/Helpers/Helpers.hpp"

		"Maia/Utilities/glTF/glTF.hpp"
		"Maia/Utilities/glTF/glTF.cpp"

		"Maia/Utilities/Math/MathHelpers.hpp"

		"Maia/Utilities/Threading/ThreadPool.hpp"

		"Maia/Utilities/Timers/PerformanceTimer.hpp"
		"Maia/Utilities/Timers/Timer.hpp"
)


install (TARGETS MaiaUtilities EXPORT MaiaUtilitiesTargets
	LIBRARY DESTINATION "lib"
	ARCHIVE DESTINATION "lib"
	RUNTIME DESTINATION "bin"
	INCLUDES DESTINATION "include"
)
install (DIRECTORY "${CMAKE_CURRENT_LIST_DIR}/Maia"
		DESTINATION "include"
        FILES_MATCHING PATTERN "*.hpp"
)
install (EXPORT MaiaUtilitiesTargets
	FILE "MaiaUtilitiesTargets.cmake"
	NAMESPACE Maia::
	DESTINATION "lib/cmake/Maia/Utilities"
)

include (CMakePackageConfigHelpers)
write_basic_package_version_file ("${CMAKE_CURRENT_BINARY_DIR}/MaiaUtilitiesConfigVersion.cmake"
	VERSION ${MaiaUtilities_VERSION}
	COMPATIBILITY SameMajorVersion
)
install (FILES "MaiaUtilitiesConfig.cmake" "${CMAKE_CURRENT_BINARY_DIR}/MaiaUtilitiesConfigVersion.cmake"
	DESTINATION "lib/cmake/Maia/Utilities"
)
