access_flag_values = (
    ("INDIRECT_COMMAND_READ", "Indirect command read", "", 0x00000001),
    ("INDEX_READ", "Index read", "", 0x00000002),
    ("VERTEX_ATTRIBUTE_READ", "Vertex attribute read", "", 0x00000004),
    ("UNIFORM_READ", "Uniform read", "", 0x00000008),
    ("INPUT_ATTACHMENT_READ", "Input attachment read", "", 0x00000010),
    ("SHADER_READ", "Shader read", "", 0x00000020),
    ("SHADER_WRITE", "Shader write", "", 0x00000040),
    ("COLOR_ATTACHMENT_READ", "Color attachment read", "", 0x00000080),
    ("COLOR_ATTACHMENT_WRITE", "Color attachment write", "", 0x00000100),
    ("DEPTH_STENCIL_ATTACHMENT_READ", "Depth stencil attachment read", "", 0x00000200),
    (
        "DEPTH_STENCIL_ATTACHMENT_WRITE",
        "Depth stencil attachment write",
        "",
        0x00000400,
    ),
    ("TRANSFER_READ", "Transfer read", "", 0x00000800),
    ("TRANSFER_WRITE", "Transfer write", "", 0x00001000),
    ("HOST_READ", "Host read", "", 0x00002000),
    ("HOST_WRITE", "Host write", "", 0x00004000),
    ("MEMORY_READ", "Memory read", "", 0x00008000),
    ("MEMORY_WRITE", "Memory write", "", 0x00010000),
)

border_color_values = (
    ("FLOAT_TRANSPARENT_BLACK", "Float transparent black", "", 0),
    ("INT_TRANSPARENT_BLACK", "Int transparent black", "", 1),
    ("FLOAT_OPAQUE_BLACK", "Float opaque black", "", 2),
    ("INT_OPAQUE_BLACK", "Int opaque black", "", 3),
    ("FLOAT_OPAQUE_WHITE", "Float opaque white", "", 4),
    ("INT_OPAQUE_WHITE", "Int opaque white", "", 5),
)

blend_factor_values = (
    ("ZERO", "Zero", "", 0),
    ("ONE", "One", "", 1),
    ("SRC_COLOR", "Src color", "", 2),
    ("ONE_MINUS_SRC_COLOR", "One minus src color", "", 3),
    ("DST_COLOR", "Dst color", "", 4),
    ("ONE_MINUS_DST_COLOR", "One minus dst color", "", 5),
    ("SRC_ALPHA", "Src alpha", "", 6),
    ("ONE_MINUS_SRC_ALPHA", "One minus src alpha", "", 7),
    ("DST_ALPHA", "Dst alpha", "", 8),
    ("ONE_MINUS_DST_ALPHA", "One minus dst alpha", "", 9),
    ("CONSTANT_COLOR", "Constant color", "", 10),
    ("ONE_MINUS_CONSTANT_COLOR", "One minus constant color", "", 11),
    ("CONSTANT_ALPHA", "Constant alpha", "", 12),
    ("ONE_MINUS_CONSTANT_ALPHA", "One minus constant alpha", "", 13),
    ("SRC_ALPHA_SATURATE", "Src alpha saturate", "", 14),
    ("SRC1_COLOR", "Src1 color", "", 15),
    ("ONE_MINUS_SRC1_COLOR", "One minus src1 color", "", 16),
    ("SRC1_ALPHA", "Src1 alpha", "", 17),
    ("ONE_MINUS_SRC1_ALPHA", "One minus src1 alpha", "", 18),
)

blend_operation_values = (
    ("ADD", "Add", "", 0),
    ("SUBTRACT", "Subtract", "", 1),
    ("REVERSE_SUBTRACT", "Reverse_subtract", "", 2),
    ("MIN", "Min", "", 3),
    ("MAX", "Max", "", 4),
)

buffer_create_flags_values = (
    ("SPARSE_BINDING_BIT", "sparse_binding_bit", "", 0x00000001),
    ("SPARSE_RESIDENCY_BIT", "sparse_residency_bit", "", 0x00000002),
    ("SPARSE_ALIASED_BIT", "sparse_aliased_bit", "", 0x00000004),
    ("PROTECTED_BIT", "protected_bit", "", 0x00000008),
    (
        "DEVICE_ADDRESS_CAPTURE_REPLAY_BIT",
        "device_address_capture_replay_bit",
        "",
        0x00000010,
    ),
)

buffer_usage_flags_values = (
    ("TRANSFER_SRC_BIT", "transfer_src_bit", "", 0x00000001),
    ("TRANSFER_DST_BIT", "transfer_dst_bit", "", 0x00000002),
    ("UNIFORM_TEXEL_BUFFER_BIT", "uniform_texel_buffer_bit", "", 0x00000004),
    ("STORAGE_TEXEL_BUFFER_BIT", "storage_texel_buffer_bit", "", 0x00000008),
    ("UNIFORM_BUFFER_BIT", "uniform_buffer_bit", "", 0x00000010),
    ("STORAGE_BUFFER_BIT", "storage_buffer_bit", "", 0x00000020),
    ("INDEX_BUFFER_BIT", "index_buffer_bit", "", 0x00000040),
    ("VERTEX_BUFFER_BIT", "vertex_buffer_bit", "", 0x00000080),
    ("INDIRECT_BUFFER_BIT", "indirect_buffer_bit", "", 0x00000100),
    ("SHADER_DEVICE_ADDRESS_BIT", "shader_device_address_bit", "", 0x00020000),
    ("VIDEO_DECODE_SRC_BIT_KHR", "video_decode_src_bit_khr", "", 0x00002000),
    ("VIDEO_DECODE_DST_BIT_KHR", "video_decode_dst_bit_khr", "", 0x00004000),
    (
        "ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR",
        "acceleration_structure_build_input_read_only_bit_khr",
        "",
        0x00080000,
    ),
    (
        "ACCELERATION_STRUCTURE_STORAGE_BIT_KHR",
        "acceleration_structure_storage_bit_khr",
        "",
        0x00100000,
    ),
    ("SHADER_BINDING_TABLE_BIT_KHR", "shader_binding_table_bit_khr", "", 0x00000400),
    ("VIDEO_ENCODE_DST_BIT_KHR", "video_encode_dst_bit_khr", "", 0x00008000),
    ("VIDEO_ENCODE_SRC_BIT_KHR", "video_encode_src_bit_khr", "", 0x00010000),
)

color_component_flag_values = (
    ("R", "R", "", 0x00000001),
    ("G", "G", "", 0x00000002),
    ("B", "B", "", 0x00000004),
    ("A", "A", "", 0x00000008),
)

compare_operation_values = (
    ("NEVER", "Never", "", 0),
    ("LESS", "Less", "", 1),
    ("EQUAL", "Equal", "", 2),
    ("LESS_OR_EQUAL", "Less or equal", "", 3),
    ("GREATER", "Greater", "", 4),
    ("NOT_EQUAL", "Not equal", "", 5),
    ("GREATER_OR_EQUAL", "Greater or equal", "", 6),
    ("ALWAYS", "Always", "", 7),
)

component_swizzle_values = (
    ("IDENTITY", "Identity", "", 0),
    ("ZERO", "Zero", "", 1),
    ("ONE", "One", "", 2),
    ("R", "R", "", 3),
    ("G", "G", "", 4),
    ("B", "B", "", 5),
    ("A", "A", "", 6),
)

cull_modes = (
    ("FRONT", "Front", "", 0x00000001),
    ("BACK", "Back", "", 0x00000002),
)

dependency_flag_values = (
    ("BY_REGION", "BY_REGION", "", 0x00000001),
    ("DEVICE_GROUP", "DEVICE_GROUP", "", 0x00000004),
    ("VIEW_LOCAL", "VIEW_LOCAL", "", 0x00000002),
)

descriptor_type_values = (
    ("SAMPLER", "Sampler", "", 0),
    ("COMBINED_IMAGE_SAMPLER", "Combined image sampler", "", 1),
    ("SAMPLED_IMAGE", "Sampled image", "", 2),
    ("STORAGE_IMAGE", "Storage image", "", 3),
    ("UNIFORM_TEXEL_BUFFER", "Uniform texel buffer", "", 4),
    ("STORAGE_TEXEL_BUFFER", "Storage texel buffer", "", 5),
    ("UNIFORM_BUFFER", "Uniform buffer", "", 6),
    ("STORAGE_BUFFER", "Storage buffer", "", 7),
    ("UNIFORM_BUFFER_DYNAMIC", "Uniform buffer dynamic", "", 8),
    ("STORAGE_BUFFER_DYNAMIC", "Storage buffer dynamic", "", 9),
    ("INPUT_ATTACHMENT", "Input attachment", "", 10),
    ("ACCELERATION_STRUCTURE_KHR", "Acceleration structure KHR", "", 150000),
)


def get_descriptor_type_value(value):
    if value == 150000:
        return 1000150000
    else:
        return value


dynamic_state_values = (
    ("VIEWPORT", "Viewport", "", 0),
    ("SCISSOR", "Scissor", "", 1),
    ("LINE_WIDTH", "Line width", "", 2),
    ("DEPTH_BIAS", "Depth bias", "", 3),
    ("BLEND_CONSTANTS", "Blend constants", "", 4),
    ("DEPTH_BOUNDS", "Depth bounds", "", 5),
    ("STENCIL_COMPARE_MASK", "Stencil compare mask", "", 6),
    ("STENCIL_WRITE_MASK", "Stencil write mask", "", 7),
    ("STENCIL_REFERENCE", "Stencil reference", "", 8),
)

filter_values = (
    ("NEAREST", "Nearest", "", 0),
    ("LINEAR", "Linear", "", 1),
)

format_values = (
    ("UNDEFINED", "UNDEFINED", "", 0),
    ("R4G4_UNORM_PACK8", "R4G4_UNORM_PACK8", "", 1),
    ("R4G4B4A4_UNORM_PACK16", "R4G4B4A4_UNORM_PACK16", "", 2),
    ("B4G4R4A4_UNORM_PACK16", "B4G4R4A4_UNORM_PACK16", "", 3),
    ("R5G6B5_UNORM_PACK16", "R5G6B5_UNORM_PACK16", "", 4),
    ("B5G6R5_UNORM_PACK16", "B5G6R5_UNORM_PACK16", "", 5),
    ("R5G5B5A1_UNORM_PACK16", "R5G5B5A1_UNORM_PACK16", "", 6),
    ("B5G5R5A1_UNORM_PACK16", "B5G5R5A1_UNORM_PACK16", "", 7),
    ("A1R5G5B5_UNORM_PACK16", "A1R5G5B5_UNORM_PACK16", "", 8),
    ("R8_UNORM", "R8_UNORM", "", 9),
    ("R8_SNORM", "R8_SNORM", "", 10),
    ("R8_USCALED", "R8_USCALED", "", 11),
    ("R8_SSCALED", "R8_SSCALED", "", 12),
    ("R8_UINT", "R8_UINT", "", 13),
    ("R8_SINT", "R8_SINT", "", 14),
    ("R8_SRGB", "R8_SRGB", "", 15),
    ("R8G8_UNORM", "R8G8_UNORM", "", 16),
    ("R8G8_SNORM", "R8G8_SNORM", "", 17),
    ("R8G8_USCALED", "R8G8_USCALED", "", 18),
    ("R8G8_SSCALED", "R8G8_SSCALED", "", 19),
    ("R8G8_UINT", "R8G8_UINT", "", 20),
    ("R8G8_SINT", "R8G8_SINT", "", 21),
    ("R8G8_SRGB", "R8G8_SRGB", "", 22),
    ("R8G8B8_UNORM", "R8G8B8_UNORM", "", 23),
    ("R8G8B8_SNORM", "R8G8B8_SNORM", "", 24),
    ("R8G8B8_USCALED", "R8G8B8_USCALED", "", 25),
    ("R8G8B8_SSCALED", "R8G8B8_SSCALED", "", 26),
    ("R8G8B8_UINT", "R8G8B8_UINT", "", 27),
    ("R8G8B8_SINT", "R8G8B8_SINT", "", 28),
    ("R8G8B8_SRGB", "R8G8B8_SRGB", "", 29),
    ("B8G8R8_UNORM", "B8G8R8_UNORM", "", 30),
    ("B8G8R8_SNORM", "B8G8R8_SNORM", "", 31),
    ("B8G8R8_USCALED", "B8G8R8_USCALED", "", 32),
    ("B8G8R8_SSCALED", "B8G8R8_SSCALED", "", 33),
    ("B8G8R8_UINT", "B8G8R8_UINT", "", 34),
    ("B8G8R8_SINT", "B8G8R8_SINT", "", 35),
    ("B8G8R8_SRGB", "B8G8R8_SRGB", "", 36),
    ("R8G8B8A8_UNORM", "R8G8B8A8_UNORM", "", 37),
    ("R8G8B8A8_SNORM", "R8G8B8A8_SNORM", "", 38),
    ("R8G8B8A8_USCALED", "R8G8B8A8_USCALED", "", 39),
    ("R8G8B8A8_SSCALED", "R8G8B8A8_SSCALED", "", 40),
    ("R8G8B8A8_UINT", "R8G8B8A8_UINT", "", 41),
    ("R8G8B8A8_SINT", "R8G8B8A8_SINT", "", 42),
    ("R8G8B8A8_SRGB", "R8G8B8A8_SRGB", "", 43),
    ("B8G8R8A8_UNORM", "B8G8R8A8_UNORM", "", 44),
    ("B8G8R8A8_SNORM", "B8G8R8A8_SNORM", "", 45),
    ("B8G8R8A8_USCALED", "B8G8R8A8_USCALED", "", 46),
    ("B8G8R8A8_SSCALED", "B8G8R8A8_SSCALED", "", 47),
    ("B8G8R8A8_UINT", "B8G8R8A8_UINT", "", 48),
    ("B8G8R8A8_SINT", "B8G8R8A8_SINT", "", 49),
    ("B8G8R8A8_SRGB", "B8G8R8A8_SRGB", "", 50),
    ("A8B8G8R8_UNORM_PACK32", "A8B8G8R8_UNORM_PACK32", "", 51),
    ("A8B8G8R8_SNORM_PACK32", "A8B8G8R8_SNORM_PACK32", "", 52),
    ("A8B8G8R8_USCALED_PACK32", "A8B8G8R8_USCALED_PACK32", "", 53),
    ("A8B8G8R8_SSCALED_PACK32", "A8B8G8R8_SSCALED_PACK32", "", 54),
    ("A8B8G8R8_UINT_PACK32", "A8B8G8R8_UINT_PACK32", "", 55),
    ("A8B8G8R8_SINT_PACK32", "A8B8G8R8_SINT_PACK32", "", 56),
    ("A8B8G8R8_SRGB_PACK32", "A8B8G8R8_SRGB_PACK32", "", 57),
    ("A2R10G10B10_UNORM_PACK32", "A2R10G10B10_UNORM_PACK32", "", 58),
    ("A2R10G10B10_SNORM_PACK32", "A2R10G10B10_SNORM_PACK32", "", 59),
    ("A2R10G10B10_USCALED_PACK32", "A2R10G10B10_USCALED_PACK32", "", 60),
    ("A2R10G10B10_SSCALED_PACK32", "A2R10G10B10_SSCALED_PACK32", "", 61),
    ("A2R10G10B10_UINT_PACK32", "A2R10G10B10_UINT_PACK32", "", 62),
    ("A2R10G10B10_SINT_PACK32", "A2R10G10B10_SINT_PACK32", "", 63),
    ("A2B10G10R10_UNORM_PACK32", "A2B10G10R10_UNORM_PACK32", "", 64),
    ("A2B10G10R10_SNORM_PACK32", "A2B10G10R10_SNORM_PACK32", "", 65),
    ("A2B10G10R10_USCALED_PACK32", "A2B10G10R10_USCALED_PACK32", "", 66),
    ("A2B10G10R10_SSCALED_PACK32", "A2B10G10R10_SSCALED_PACK32", "", 67),
    ("A2B10G10R10_UINT_PACK32", "A2B10G10R10_UINT_PACK32", "", 68),
    ("A2B10G10R10_SINT_PACK32", "A2B10G10R10_SINT_PACK32", "", 69),
    ("R16_UNORM", "R16_UNORM", "", 70),
    ("R16_SNORM", "R16_SNORM", "", 71),
    ("R16_USCALED", "R16_USCALED", "", 72),
    ("R16_SSCALED", "R16_SSCALED", "", 73),
    ("R16_UINT", "R16_UINT", "", 74),
    ("R16_SINT", "R16_SINT", "", 75),
    ("R16_SFLOAT", "R16_SFLOAT", "", 76),
    ("R16G16_UNORM", "R16G16_UNORM", "", 77),
    ("R16G16_SNORM", "R16G16_SNORM", "", 78),
    ("R16G16_USCALED", "R16G16_USCALED", "", 79),
    ("R16G16_SSCALED", "R16G16_SSCALED", "", 80),
    ("R16G16_UINT", "R16G16_UINT", "", 81),
    ("R16G16_SINT", "R16G16_SINT", "", 82),
    ("R16G16_SFLOAT", "R16G16_SFLOAT", "", 83),
    ("R16G16B16_UNORM", "R16G16B16_UNORM", "", 84),
    ("R16G16B16_SNORM", "R16G16B16_SNORM", "", 85),
    ("R16G16B16_USCALED", "R16G16B16_USCALED", "", 86),
    ("R16G16B16_SSCALED", "R16G16B16_SSCALED", "", 87),
    ("R16G16B16_UINT", "R16G16B16_UINT", "", 88),
    ("R16G16B16_SINT", "R16G16B16_SINT", "", 89),
    ("R16G16B16_SFLOAT", "R16G16B16_SFLOAT", "", 90),
    ("R16G16B16A16_UNORM", "R16G16B16A16_UNORM", "", 91),
    ("R16G16B16A16_SNORM", "R16G16B16A16_SNORM", "", 92),
    ("R16G16B16A16_USCALED", "R16G16B16A16_USCALED", "", 93),
    ("R16G16B16A16_SSCALED", "R16G16B16A16_SSCALED", "", 94),
    ("R16G16B16A16_UINT", "R16G16B16A16_UINT", "", 95),
    ("R16G16B16A16_SINT", "R16G16B16A16_SINT", "", 96),
    ("R16G16B16A16_SFLOAT", "R16G16B16A16_SFLOAT", "", 97),
    ("R32_UINT", "R32_UINT", "", 98),
    ("R32_SINT", "R32_SINT", "", 99),
    ("R32_SFLOAT", "R32_SFLOAT", "", 100),
    ("R32G32_UINT", "R32G32_UINT", "", 101),
    ("R32G32_SINT", "R32G32_SINT", "", 102),
    ("R32G32_SFLOAT", "R32G32_SFLOAT", "", 103),
    ("R32G32B32_UINT", "R32G32B32_UINT", "", 104),
    ("R32G32B32_SINT", "R32G32B32_SINT", "", 105),
    ("R32G32B32_SFLOAT", "R32G32B32_SFLOAT", "", 106),
    ("R32G32B32A32_UINT", "R32G32B32A32_UINT", "", 107),
    ("R32G32B32A32_SINT", "R32G32B32A32_SINT", "", 108),
    ("R32G32B32A32_SFLOAT", "R32G32B32A32_SFLOAT", "", 109),
    ("R64_UINT", "R64_UINT", "", 110),
    ("R64_SINT", "R64_SINT", "", 111),
    ("R64_SFLOAT", "R64_SFLOAT", "", 112),
    ("R64G64_UINT", "R64G64_UINT", "", 113),
    ("R64G64_SINT", "R64G64_SINT", "", 114),
    ("R64G64_SFLOAT", "R64G64_SFLOAT", "", 115),
    ("R64G64B64_UINT", "R64G64B64_UINT", "", 116),
    ("R64G64B64_SINT", "R64G64B64_SINT", "", 117),
    ("R64G64B64_SFLOAT", "R64G64B64_SFLOAT", "", 118),
    ("R64G64B64A64_UINT", "R64G64B64A6filter_values4_UINT", "", 119),
    ("R64G64B64A64_SINT", "R64G64B64A64_SINT", "", 120),
    ("R64G64B64A64_SFLOAT", "R64G64B64A64_SFLOAT", "", 121),
    ("B10G11R11_UFLOAT_PACK32", "B10G11R11_UFLOAT_PACK32", "", 122),
    ("E5B9G9R9_UFLOAT_PACK32", "E5B9G9R9_UFLOAT_PACK32", "", 123),
    ("D16_UNORM", "D16_UNORM", "", 124),
    ("X8_D24_UNORM_PACK32", "X8_D24_UNORM_PACK32", "", 125),
    ("D32_SFLOAT", "D32_SFLOAT", "", 126),
    ("S8_UINT", "S8_UINT", "", 127),
    ("D16_UNORM_S8_UINT", "D16_UNORM_S8_UINT", "", 128),
    ("D24_UNORM_S8_UINT", "D24_UNORM_S8_UINT", "", 129),
    ("D32_SFLOAT_S8_UINT", "D32_SFLOAT_S8_UINT", "", 130),
    ("BC1_RGB_UNORM_BLOCK", "BC1_RGB_UNORM_BLOCK", "", 131),
    ("BC1_RGB_SRGB_BLOCK", "BC1_RGB_SRGB_BLOCK", "", 132),
    ("BC1_RGBA_UNORM_BLOCK", "BC1_RGBA_UNORM_BLOCK", "", 133),
    ("BC1_RGBA_SRGB_BLOCK", "BC1_RGBA_SRGB_BLOCK", "", 134),
    ("BC2_UNORM_BLOCK", "BC2_UNORM_BLOCK", "", 135),
    ("BC2_SRGB_BLOCK", "BC2_SRGB_BLOCK", "", 136),
    ("BC3_UNORM_BLOCK", "BC3_UNORM_BLOCK", "", 137),
    ("BC3_SRGB_BLOCK", "BC3_SRGB_BLOCK", "", 138),
    ("BC4_UNORM_BLOCK", "BC4_UNORM_BLOCK", "", 139),
    ("BC4_SNORM_BLOCK", "BC4_SNORM_BLOCK", "", 140),
    ("BC5_UNORM_BLOCK", "BC5_UNORM_BLOCK", "", 141),
    ("BC5_SNORM_BLOCK", "BC5_SNORM_BLOCK", "", 142),
    ("BC6H_UFLOAT_BLOCK", "BC6H_UFLOAT_BLOCK", "", 143),
    ("BC6H_SFLOAT_BLOCK", "BC6H_SFLOAT_BLOCK", "", 144),
    ("BC7_UNORM_BLOCK", "BC7_UNORM_BLOCK", "", 145),
    ("BC7_SRGB_BLOCK", "BC7_SRGB_BLOCK", "", 146),
)

front_face = (
    ("COUNTER_CLOCKWISE", "Counter Clockwise", "", 0),
    ("CLOCKWISE", "Clockwise", "", 1),
)

image_aspect_flag_bits = (
    ("COLOR_BIT", "color_bit", "", 0x00000001),
    ("DEPTH_BIT", "depth_bit", "", 0x00000002),
    ("STENCIL_BIT", "stencil_bit", "", 0x00000004),
    ("METADATA_BIT", "metadata_bit", "", 0x00000008),
    ("PLANE_0_BIT", "plane_0_bit", "", 0x00000010),
    ("PLANE_1_BIT", "plane_1_bit", "", 0x00000020),
    ("PLANE_2_BIT", "plane_2_bit", "", 0x00000040),
    ("NONE_KHR", "none_khr", "", 0),
)

image_create_flags = (
    ("SPARSE_BINDING_BIT", "sparse_binding_bit", "", 0x00000001),
    ("SPARSE_RESIDENCY_BIT", "sparse_residency_bit", "", 0x00000002),
    ("SPARSE_ALIASED_BIT", "sparse_aliased_bit", "", 0x00000004),
    ("MUTABLE_FORMAT_BIT", "mutable_format_bit", "", 0x00000008),
    ("CUBE_COMPATIBLE_BIT", "cube_compatible_bit", "", 0x00000010),
    ("ALIAS_BIT", "alias_bit", "", 0x00000400),
    (
        "SPLIT_INSTANCE_BIND_REGIONS_BIT",
        "split_instance_bind_regions_bit",
        "",
        0x00000040,
    ),
    ("2D_ARRAY_COMPATIBLE_BIT", "2d_array_compatible_bit", "", 0x00000020),
    (
        "BLOCK_TEXEL_VIEW_COMPATIBLE_BIT",
        "block_texel_view_compatible_bit",
        "",
        0x00000080,
    ),
    ("EXTENDED_USAGE_BIT", "extended_usage_bit", "", 0x00000100),
    ("PROTECTED_BIT", "protected_bit", "", 0x00000800),
    ("DISJOINT_BIT", "disjoint_bit", "", 0x00000200),
)

image_layout_values = (
    ("UNDEFINED", "Undefined", "", 0),
    ("GENERAL", "General", "", 1),
    ("COLOR_ATTACHMENT_OPTIMAL", "Color_attachment_optimal", "", 2),
    ("DEPTH_STENCIL_ATTACHMENT_OPTIMAL", "Depth_stencil_attachment_optimal", "", 3),
    ("DEPTH_STENCIL_READ_ONLY_OPTIMAL", "Depth_stencil_read_only_optimal", "", 4),
    ("SHADER_READ_ONLY_OPTIMAL", "Shader_read_only_optimal", "", 5),
    ("TRANSFER_SRC_OPTIMAL", "Transfer_src_optimal", "", 6),
    ("TRANSFER_DST_OPTIMAL", "Transfer_dst_optimal", "", 7),
    ("PREINITIALIZED", "Preinitialized", "", 8),
    (
        "DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL",
        "Depth_read_only_stencil_attachment_optimal",
        "",
        109,
    ),
    (
        "DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL",
        "Depth_attachment_stencil_read_only_optimal",
        "",
        110,
    ),
    ("DEPTH_ATTACHMENT_OPTIMAL", "Depth_attachment_optimal", "", 111),
    ("DEPTH_READ_ONLY_OPTIMAL", "Depth_read_only_optimal", "", 112),
    ("STENCIL_ATTACHMENT_OPTIMAL", "Stencil_attachment_optimal", "", 113),
    ("STENCIL_READ_ONLY_OPTIMAL", "Stencil_read_only_optimal", "", 114),
    ("PRESENT_SRC_KHR", "Present_src_khr", "", 115),
    ("SHARED_PRESENT_KHR", "Shared_present_khr", "", 116),
)

image_type_values = (
    ("1D", "1D", "", 0),
    ("2D", "2D", "", 1),
    ("3D", "3D", "", 2),
)

image_layout_values_to_int = {
    "UNDEFINED": 0,
    "GENERAL": 1,
    "COLOR_ATTACHMENT_OPTIMAL": 2,
    "DEPTH_STENCIL_ATTACHMENT_OPTIMAL": 3,
    "DEPTH_STENCIL_READ_ONLY_OPTIMAL": 4,
    "SHADER_READ_ONLY_OPTIMAL": 5,
    "TRANSFER_SRC_OPTIMAL": 6,
    "TRANSFER_DST_OPTIMAL": 7,
    "PREINITIALIZED": 8,
    "DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL": 1000117000,
    "DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL": 1000117001,
    "DEPTH_ATTACHMENT_OPTIMAL": 1000241000,
    "DEPTH_READ_ONLY_OPTIMAL": 1000241001,
    "STENCIL_ATTACHMENT_OPTIMAL": 1000241002,
    "STENCIL_READ_ONLY_OPTIMAL": 1000241003,
    "PRESENT_SRC_KHR": 1000001002,
    "SHARED_PRESENT_KHR": 1000111000,
}

image_view_type_values = (
    ("1D", "1D", "", 0),
    ("2D", "2D", "", 1),
    ("3D", "3D", "", 2),
    ("CUBE", "Cube", "", 3),
    ("1D_ARRAY", "1D array", "", 4),
    ("2D_ARRAY", "2D array", "", 5),
    ("CUBE_ARRAY", "Cube array", "", 6),
)

image_usage_flag_bits = (
    ("TRANSFER_SRC_BIT", "transfer_src_bit", "", 0x00000001),
    ("TRANSFER_DST_BIT", "transfer_dst_bit", "", 0x00000002),
    ("SAMPLED_BIT", "sampled_bit", "", 0x00000004),
    ("STORAGE_BIT", "storage_bit", "", 0x00000008),
    ("COLOR_ATTACHMENT_BIT", "color_attachment_bit", "", 0x00000010),
    ("DEPTH_STENCIL_ATTACHMENT_BIT", "depth_stencil_attachment_bit", "", 0x00000020),
    ("TRANSIENT_ATTACHMENT_BIT", "transient_attachment_bit", "", 0x00000040),
    ("INPUT_ATTACHMENT_BIT", "input_attachment_bit", "", 0x00000080),
    ("VIDEO_DECODE_DST_BIT_KHR", "video_decode_dst_bit_khr", "", 0x00000400),
    ("VIDEO_DECODE_SRC_BIT_KHR", "video_decode_src_bit_khr", "", 0x00000800),
    ("VIDEO_DECODE_DPB_BIT_KHR", "video_decode_dpb_bit_khr", "", 0x00001000),
    (
        "FRAGMENT_SHADING_RATE_ATTACHMENT_BIT_KHR",
        "fragment_shading_rate_attachment_bit_khr",
        "",
        0x00000100,
    ),
    ("VIDEO_ENCODE_DST_BIT_KHR", "video_encode_dst_bit_khr", "", 0x00002000),
    ("VIDEO_ENCODE_SRC_BIT_KHR", "video_encode_src_bit_khr", "", 0x00004000),
    ("VIDEO_ENCODE_DPB_BIT_KHR", "video_encode_dpb_bit_khr", "", 0x00008000),
)

logic_operation_values = (
    ("CLEAR", "Clear", "", 0),
    ("AND", "And", "", 1),
    ("AND_REVERSE", "And reverse", "", 2),
    ("COPY", "Copy", "", 3),
    ("AND_INVERTED", "And inverted", "", 4),
    ("NO_OP", "No_op", "", 5),
    ("XOR", "Xor", "", 6),
    ("OR", "Or", "", 7),
    ("NOR", "Nor", "", 8),
    ("EQUIVALENT", "Equivalent", "", 9),
    ("INVERT", "Invert", "", 10),
    ("OR_REVERSE", "Or reverse", "", 11),
    ("COPY_INVERTED", "Copy inverted", "", 12),
    ("OR_INVERTED", "Or inverted", "", 13),
    ("NAND", "Nand", "", 14),
    ("SET", "Set", "", 15),
)

pipeline_bind_point_values = (
    ("GRAPHICS", "Graphics", "", 0),
    ("COMPUTE", "Compute", "", 1),
    ("RAY_TRACING_KHR", "Ray", "", 165000),
)


def get_pipeline_bind_point_value(value):
    if value == 165000:
        return 1000165000
    else:
        return value


pipeline_stage_flag_values = (
    ("TOP_OF_PIPE", "top_of_pipe", "", 0x00000001),
    ("DRAW_INDIRECT", "draw_indirect", "", 0x00000002),
    ("VERTEX_INPUT", "vertex_input", "", 0x00000004),
    ("VERTEX_SHADER", "vertex_shader", "", 0x00000008),
    (
        "TESSELLATION_CONTROL_SHADER",
        "tessellation_control_shader",
        "",
        0x00000010,
    ),
    (
        "TESSELLATION_EVALUATION_SHADER",
        "tessellation_evaluation_shader",
        "",
        0x00000020,
    ),
    ("GEOMETRY_SHADER", "geometry_shader", "", 0x00000040),
    ("FRAGMENT_SHADER", "fragment_shader", "", 0x00000080),
    ("EARLY_FRAGMENT_TESTS", "early_fragment_tests", "", 0x00000100),
    ("LATE_FRAGMENT_TESTS", "late_fragment_tests", "", 0x00000200),
    ("COLOR_ATTACHMENT_OUTPUT", "color_attachment_output", "", 0x00000400),
    ("COMPUTE_SHADER", "compute_shader", "", 0x00000800),
    ("TRANSFER", "transfer", "", 0x00001000),
    ("BOTTOM_OF_PIPE", "bottom_of_pipe", "", 0x00002000),
    ("HOST", "host", "", 0x00004000),
    ("ALL_GRAPHICS", "all_graphics", "", 0x00008000),
    ("ALL_COMMANDS", "all_commands", "", 0x00010000),
    ("VIDEO_DECODE_KHR", "video_decode_khr", "", 0x04000000),
    ("VIDEO_ENCODE_KHR", "video_encode_khr", "", 0x08000000),
    ("TRANSFORM_FEEDBACK_EXT", "transform_feedback_ext", "", 0x01000000),
    ("CONDITIONAL_RENDERING_EXT", "conditional_rendering_ext", "", 0x00040000),
    ("COMMAND_PREPROCESS_NV", "command_preprocess_nv", "", 0x00020000),
    (
        "FRAGMENT_SHADING_RATE_ATTACHMENT_KHR",
        "fragment_shading_rate_attachment_khr",
        "",
        0x00400000,
    ),
    (
        "ACCELERATION_STRUCTURE_BUILD_KHR",
        "acceleration_structure_build_khr",
        "",
        0x02000000,
    ),
    ("RAY_TRACING_SHADER_KHR", "ray_tracing_shader_khr", "", 0x00200000),
    (
        "FRAGMENT_DENSITY_PROCESS_EXT",
        "fragment_density_process_ext",
        "",
        0x00800000,
    ),
    ("TASK_SHADER_NV", "task_shader_nv", "", 0x00080000),
    ("MESH_SHADER_NV", "mesh_shader_nv", "", 0x00100000),
)

pipeline_stage_flag_values_2 = (
    ("COPY", "copy", "", 0x1),
    ("RESOLVE", "resolve", "", 0x2),
    ("BLIT", "blit", "", 0x4),
    ("CLEAR", "clear", "", 0x8),
    ("INDEX_INPUT", "index_input", "", 0x10),
    ("VERTEX_ATTRIBUTE_INPUT", "vertex_attribute_input", "", 0x20),
    (
        "PRE_RASTERIZATION_SHADERS",
        "pre_rasterization_shaders",
        "",
        0x40,
    ),
)


def get_pipeline_stage_flags_value(stages_1, stages_2) -> int:
    return stages_1 + (stages_2 << 32)


polygon_modes = (
    ("FILL", "Fill", "", 0),
    ("LINE", "Line", "", 1),
    ("POINT", "Point", "", 2),
)

sample_count_flag_bits = (
    ("1_BIT", "1_BIT", "", 0x00000001),
    ("2_BIT", "2_BIT", "", 0x00000002),
    ("4_BIT", "4_BIT", "", 0x00000004),
    ("8_BIT", "8_BIT", "", 0x00000008),
    ("16_BIT", "16_BIT", "", 0x00000010),
    ("32_BIT", "32_BIT", "", 0x00000020),
    ("64_BIT", "64_BIT", "", 0x00000040),
)

sampler_address_move_values = (
    ("REPEAT", "Repeat", "", 0),
    ("MIRRORED_REPEAT", "Mirrored repeat", "", 1),
    ("CLAMP_TO_EDGE", "Clamp to edge", "", 2),
    ("CLAMP_TO_BORDER", "Clamp to border", "", 3),
    ("MIRROR_CLAMP_TO_EDGE", "Mirror clamp to edge", "", 4),
)

sampler_mipmap_mode_values = (
    ("NEAREST", "Nearest", "", 0),
    ("LINEAR", "Linear", "", 1),
)

shader_stage_flag_values = (
    ("VERTEX", "Vertex", "", 0x00000001),
    ("TESSELLATION_CONTROL", "Tessellation control", "", 0x00000002),
    ("TESSELLATION_EVALUATION", "Tessellation evaluation", "", 0x00000004),
    ("GEOMETRY", "Geometry", "", 0x00000008),
    ("FRAGMENT", "Fragment", "", 0x00000010),
    ("COMPUTE", "Compute", "", 0x00000020),
    ("AMPLIFICATION", "Amplification", "", 0x00000040),
    ("MESH", "Mesh", "", 0x00000080),
    ("RAYGEN", "Ray generation", "", 0x00000100),
    ("ANY_HIT", "Any hit", "", 0x00000200),
    ("CLOSEST_HIT", "Closest hit", "", 0x00000400),
    ("MISS_HIT", "Miss hit", "", 0x00000800),
    ("INTERSECTION", "Intersection", "", 0x00001000),
    ("CALLABLE", "Callable", "", 0x00002000),
)


stencil_operation_values = (
    ("KEEP", "Keep", "", 0),
    ("ZERO", "Zero", "", 1),
    ("REPLACE", "Replace", "", 2),
    ("INCREMENT_AND_CLAMP", "Increment and clamp", "", 3),
    ("DECREMENT_AND_CLAMP", "Decrement and clamp", "", 4),
    ("INVERT", "Invert", "", 5),
    ("INCREMENT_AND_WRAP", "Increment and wrap", "", 6),
    ("DECREMENT_AND_WRAP", "Decrement and wrap", "", 7),
)

tiling_values = (
    ("OPTIMAL", "optimal", "", 0),
    ("LINEAR", "linear", "", 1),
)
