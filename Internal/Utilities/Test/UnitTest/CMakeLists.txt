project (MaiaUtilitiesUnitTest)

add_executable (MaiaUtilitiesUnitTest)
add_executable (Maia::Utilities::UnitTest ALIAS MaiaUtilitiesUnitTest)

target_compile_features (MaiaUtilitiesUnitTest PRIVATE cxx_std_17)

target_include_directories(MaiaUtilitiesUnitTest 
	PRIVATE
		"${CMAKE_CURRENT_LIST_DIR}"
)

target_link_libraries (MaiaUtilitiesUnitTest PRIVATE Maia::Utilities)

find_package (Catch2 REQUIRED)
target_link_libraries (MaiaUtilitiesUnitTest PRIVATE Catch2::Catch2)

target_sources (MaiaUtilitiesUnitTest 
	PRIVATE

		"main.cpp"

		#"Allocators/Buddy_allocator_test.cpp"
		#"Allocators/Forward_allocator_test.cpp"

		#"Containers/Pools/ContiguousMemoryPoolTest.cpp"
		#"Containers/Pools/MemoryPoolTest.cpp"
		#"Containers/Pools/PointedContiguousMemoryPoolTest.cpp"

		"Containers/Chunks/Memory_chunk.test.cpp"
		"Containers/Chunks/Memory_chunks.test.cpp"

		#"Math/MathHelpersTest.cpp"

		#"Threading/ThreadPoolTest.cpp"

)

include (Catch)
catch_discover_tests (MaiaUtilitiesUnitTest)
