add_library (MaiaMythologyImage)
add_library (Maia::Mythology::Image ALIAS MaiaMythologyImage)

target_link_libraries (MaiaMythologyImage PRIVATE Standard_library Stb_module)

target_module (
    MaiaMythologyImage
    PUBLIC
    "mythology.image"
    INTERFACE "Image.cppm"
    INTERFACE_DEPENDS "Standard_library"
    IMPLEMENTATION "Image.cpp"
    IMPLEMENTATION_DEPENDS "Standard_library" "Stb_module"
)
