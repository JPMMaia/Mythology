add_library (Eigen3_module)

set_target_properties (Eigen3_module PROPERTIES LINKER_LANGUAGE CXX)

find_package (Eigen3 3.3.7 CONFIG REQUIRED)
target_link_libraries (Eigen3_module PUBLIC Eigen3::Eigen)

set (eigen3_root "${EIGEN3_INCLUDE_DIR}")
target_header (Eigen3_module PUBLIC "eigen3_core" "${eigen3_root}/Eigen/Core")
target_header (Eigen3_module PUBLIC "eigen3_geometry" "${eigen3_root}/Eigen/Geometry")