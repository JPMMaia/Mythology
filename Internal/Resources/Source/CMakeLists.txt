project (Resources)

add_library (Resources OBJECT)
add_library (Maia::Resources ALIAS Resources)

set_target_properties (Resources PROPERTIES LINKER_LANGUAGE CXX)


include (target_resources)

target_resources (Resources
    PRIVATE
        "box.gltf"
        "gizmo.gltf"
)
