import bpy
import typing

from bpy.types import NodeTree, Node, NodeSocket

# Implementation of custom nodes from Python


# Derived from the NodeTree base type, similar to Menu, Operator, Panel, etc.
class MyCustomTree(NodeTree):
    # Description string
    '''A custom node tree type that will show up in the editor type list'''
    # Optional identifier string. If not explicitly defined, the python class name is used.
    bl_idname = 'CustomTreeType'
    # Label for nice name display
    bl_label = "Custom Node Tree"
    # Icon identifier
    bl_icon = 'NODETREE'


# Custom socket type
class MyCustomSocket(NodeSocket):
    # Description string
    '''Custom node socket type'''
    # Optional identifier string. If not explicitly defined, the python class name is used.
    bl_idname = 'CustomSocketType'
    # Label for nice name display
    bl_label = "Custom Node Socket"

    # Enum items list
    my_items = (
        ('DOWN', "Down", "Where your feet are"),
        ('UP', "Up", "Where your head should be"),
        ('LEFT', "Left", "Not right"),
        ('RIGHT', "Right", "Not left"),
    )

    my_enum_prop: bpy.props.EnumProperty(
        name="Direction",
        description="Just an example",
        items=my_items,
        default='UP',
    )

    # Optional function for drawing the socket input value
    def draw(self, context, layout, node, text):
        if self.is_output or self.is_linked:
            layout.label(text=text)
        else:
            layout.prop(self, "my_enum_prop", text=text)

    # Socket color
    def draw_color(self, context, node):
        return (1.0, 0.0, 0.0, 1.0)


class AccessFlagsNodeSocket(bpy.types.NodeSocket):
    
    bl_label = "Access Flags Socket"

    enum_values = (
        ("INDIRECT_COMMAND_READ", "INDIRECT_COMMAND_READ", "", 0x00000001),
        ("INDEX_READ", "INDEX_READ", "", 0x00000002),
        ("VERTEX_ATTRIBUTE_READ", "VERTEX_ATTRIBUTE_READ", "", 0x00000004),
        ("UNIFORM_READ", "UNIFORM_READ", "", 0x00000008),
        ("INPUT_ATTACHMENT_READ", "INPUT_ATTACHMENT_READ", "", 0x00000010),
        ("SHADER_READ", "SHADER_READ", "", 0x00000020),
        ("SHADER_WRITE", "SHADER_WRITE", "", 0x00000040),
        ("COLOR_ATTACHMENT_READ", "COLOR_ATTACHMENT_READ", "", 0x00000080),
        ("COLOR_ATTACHMENT_WRITE", "COLOR_ATTACHMENT_WRITE", "", 0x00000100),
        ("DEPTH_STENCIL_ATTACHMENT_READ", "DEPTH_STENCIL_ATTACHMENT_READ", "", 0x00000200),
        ("DEPTH_STENCIL_ATTACHMENT_WRITE", "DEPTH_STENCIL_ATTACHMENT_WRITE", "", 0x00000400),
        ("TRANSFER_READ", "TRANSFER_READ", "", 0x00000800),
        ("TRANSFER_WRITE", "TRANSFER_WRITE", "", 0x00001000),
        ("HOST_READ", "HOST_READ", "", 0x00002000),
        ("HOST_WRITE", "HOST_WRITE", "", 0x00004000),
        ("MEMORY_READ", "MEMORY_READ", "", 0x00008000),
        ("MEMORY_WRITE", "MEMORY_WRITE", "", 0x00010000),
    )

    default_value: bpy.props.EnumProperty(
        name="Access Flags",
        description="Access Flags",
        items=enum_values,
        options={"ANIMATABLE", "ENUM_FLAG"}
    )

    def get_value(self) -> typing.Union[int, str]:
        return self.get('default_value', 0)

    def draw(self, context, layout, node, text):
        if self.is_output or self.is_linked:
            layout.label(text=text)
        else:
            layout.prop(self, "default_value", text=text)

    def draw_color(self, context, node):
        return (1.0, 0.0, 0.0, 1.0)

class AttachmentNodeSocket(bpy.types.NodeSocket):
    
    bl_label = "Attachment Node Socket"

    def draw(self, context, layout, node, text):
        layout.label(text=text)

    def draw_color(self, context, node):
        return (0.0, 1.0, 0.0, 1.0)

class AttachmentReferenceNodeSocket(bpy.types.NodeSocket):
    
    bl_label = "Attachment Reference Node Socket"

    def draw(self, context, layout, node, text):
        layout.label(text=text)

    def draw_color(self, context, node):
        return (0.0, 0.0, 1.0, 1.0)

class DependencyFlagsNodeSocket(bpy.types.NodeSocket):
    
    bl_label = "Dependency Flags Socket"

    enum_values = (
        ("BY_REGION", "BY_REGION", "", 0x00000001),
        ("DEVICE_GROUP", "DEVICE_GROUP", "", 0x00000004),
        ("VIEW_LOCAL", "VIEW_LOCAL", "", 0x00000002),
    )

    default_value: bpy.props.EnumProperty(
        name="Dependency Flags",
        description="Dependency Flags",
        items=enum_values,
        options={"ANIMATABLE", "ENUM_FLAG"},
    )

    def get_value(self) -> typing.Union[int, str]:
        return self.get('default_value', 0)

    def draw(self, context, layout, node, text):
        if self.is_output or self.is_linked:
            layout.label(text=text)
        else:
            layout.prop(self, "default_value", text=text)

    def draw_color(self, context, node):
        return (1.0, 0.5, 0.0, 1.0)

class FormatNodeSocket(bpy.types.NodeSocket):
    
    bl_label = "Format Node Socket"

    enum_values = (
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
        ("R64G64B64A64_UINT", "R64G64B64A64_UINT", "", 119),
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

    default_value: bpy.props.EnumProperty(
        name="Format",
        description="Format",
        items=enum_values,
        default='UNDEFINED',
    )

    def get_value(self) -> typing.Union[int, str]:
        return self.get('default_value', 0)

    def draw(self, context, layout, node, text):
        if self.is_output or self.is_linked:
            layout.label(text=text)
        else:
            layout.prop(self, "default_value", text=text)

    def draw_color(self, context, node):
        return (0.5, 1.0, 0.0, 1.0)


class SampleCountNodeSocket(bpy.types.NodeSocket):
    
    bl_label = "Format Node Socket"

    enum_values = (
        ("1", "1", "", 1),
        ("2", "2", "", 2),
        ("4", "4", "", 4),
        ("8", "8", "", 8),
        ("16", "16", "", 16),
        ("32", "32", "", 32),
        ("64", "64", "", 64),
    )

    default_value: bpy.props.EnumProperty(
        name="Sample count",
        description="Sample count",
        items=enum_values,
        default="1",
    )

    def get_value(self) -> typing.Union[int, str]:
        return self.get('default_value', 1)

    def draw(self, context, layout, node, text):
        if self.is_output or self.is_linked:
            layout.label(text=text)
        else:
            layout.prop(self, "default_value", text=text)

    def draw_color(self, context, node):
        return (0.0, 0.5, 1.0, 1.0)

class LoadOperationNodeSocket(bpy.types.NodeSocket):
    
    bl_label = "Load operation Socket"

    enum_values = (
        ("LOAD", "Load", "", 0),
        ("CLEAR", "Clear", "", 1),
        ("DONT_CARE", "Don't care", "", 2),
    )

    default_value: bpy.props.EnumProperty(
        name="Load operation",
        description="Load operation",
        items=enum_values,
        default="LOAD",
    )

    def get_value(self) -> typing.Union[int, str]:
        return self.get('default_value', 2)

    def draw(self, context, layout, node, text):
        if self.is_output or self.is_linked:
            layout.label(text=text)
        else:
            layout.prop(self, "default_value", text=text)

    def draw_color(self, context, node):
        return (1.0, 0.5, 0.5, 1.0)


class StoreOperationNodeSocket(bpy.types.NodeSocket):
    
    bl_label = "Store operation Socket"

    enum_values = (
        ("STORE", "Store", "", 0),
        ("DONT_CARE", "Don't care", "", 1),
    )

    default_value: bpy.props.EnumProperty(
        name="Store operation",
        description="Store operation",
        items=enum_values,
        default="STORE",
    )

    def get_value(self) -> typing.Union[int, str]:
        return self.get('default_value', 1)

    def draw(self, context, layout, node, text):
        if self.is_output or self.is_linked:
            layout.label(text=text)
        else:
            layout.prop(self, "default_value", text=text)

    def draw_color(self, context, node):
        return (1.0, 1.0, 0.5, 1.0)

class SubpassNodeSocket(bpy.types.NodeSocket):
    
    bl_label = "Subpass Node Socket"

    def draw(self, context, layout, node, text):
        layout.label(text=text)

    def draw_color(self, context, node):
        return (1.0, 0.25, 0.40, 1.0)

class ImageLayoutNodeSocket(bpy.types.NodeSocket):
    
    bl_label = "Image layout Socket"

    enum_values = (
        ("UNDEFINED", "UNDEFINED", "", 0),
        ("GENERAL", "GENERAL", "", 1),
        ("COLOR_ATTACHMENT_OPTIMAL", "COLOR_ATTACHMENT_OPTIMAL", "", 2),
        ("DEPTH_STENCIL_ATTACHMENT_OPTIMAL", "DEPTH_STENCIL_ATTACHMENT_OPTIMAL", "", 3),
        ("DEPTH_STENCIL_READ_ONLY_OPTIMAL", "DEPTH_STENCIL_READ_ONLY_OPTIMAL", "", 4),
        ("SHADER_READ_ONLY_OPTIMAL", "SHADER_READ_ONLY_OPTIMAL", "", 5),
        ("TRANSFER_SRC_OPTIMAL", "TRANSFER_SRC_OPTIMAL", "", 6),
        ("TRANSFER_DST_OPTIMAL", "TRANSFER_DST_OPTIMAL", "", 7),
        ("PREINITIALIZED", "PREINITIALIZED", "", 8),
        ("DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL", "DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL", "", 1000117000),
        ("DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL", "DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL", "", 1000117001),
        ("DEPTH_ATTACHMENT_OPTIMAL", "DEPTH_ATTACHMENT_OPTIMAL", "", 1000241000),
        ("DEPTH_READ_ONLY_OPTIMAL", "DEPTH_READ_ONLY_OPTIMAL", "", 1000241001),
        ("STENCIL_ATTACHMENT_OPTIMAL", "STENCIL_ATTACHMENT_OPTIMAL", "", 1000241002),
        ("STENCIL_READ_ONLY_OPTIMAL", "STENCIL_READ_ONLY_OPTIMAL", "", 1000241003),
    )

    default_value: bpy.props.EnumProperty(
        name="Image layout",
        description="Image layout",
        items=enum_values,
        default="UNDEFINED",
    )

    def get_value(self) -> typing.Union[int, str]:
        return self.get('default_value', 0)

    def draw(self, context, layout, node, text):
        if self.is_output or self.is_linked:
            layout.label(text=text)
        else:
            layout.prop(self, "default_value", text=text)

    def draw_color(self, context, node):
        return (0.75, 0.25, 0.5, 1.0)


class PipelineBindPointNodeSocket(bpy.types.NodeSocket):
    
    bl_label = "Pipeline Bind Point Node Socket"

    enum_values = (
        ("GRAPHICS", "Graphics", "", 0),
        ("COMPUTE", "Compute", "", 1),
    )

    default_value: bpy.props.EnumProperty(
        name="Pipeline Bind Point",
        description="Pipeline Bind Point",
        items=enum_values,
        default="GRAPHICS",
    )

    def get_value(self) -> typing.Union[int, str]:
        return self.get('default_value', 0)

    def draw(self, context, layout, node, text):
        if self.is_output or self.is_linked:
            layout.label(text=text)
        else:
            layout.prop(self, "default_value", text=text)

    def draw_color(self, context, node):
        return (0.25, 0.5, 0.75, 1.0)


class PipelineStageFlagsNodeSocket(bpy.types.NodeSocket):
    
    bl_label = "Pipeline Stage Flags Node Socket"

    enum_values = (
        ("TOP_OF_PIPE", "TOP_OF_PIPE", "", 0x00000001),
        ("DRAW_INDIRECT", "DRAW_INDIRECT", "", 0x00000002),
        ("VERTEX_INPUT", "VERTEX_INPUT", "", 0x00000004),
        ("VERTEX_SHADER", "VERTEX_SHADER", "", 0x00000008),
        ("TESSELLATION_CONTROL_SHADER", "TESSELLATION_CONTROL_SHADER", "", 0x00000010),
        ("TESSELLATION_EVALUATION_SHADER", "TESSELLATION_EVALUATION_SHADER", "", 0x00000020),
        ("GEOMETRY_SHADER", "GEOMETRY_SHADER", "", 0x00000040),
        ("FRAGMENT_SHADER", "FRAGMENT_SHADER", "", 0x00000080),
        ("EARLY_FRAGMENT_TESTS", "EARLY_FRAGMENT_TESTS", "", 0x00000100),
        ("LATE_FRAGMENT_TESTS", "LATE_FRAGMENT_TESTS", "", 0x00000200),
        ("COLOR_ATTACHMENT_OUTPUT", "COLOR_ATTACHMENT_OUTPUT", "", 0x00000400),
        ("COMPUTE_SHADER", "COMPUTE_SHADER", "", 0x00000800),
        ("TRANSFER", "TRANSFER", "", 0x00001000),
        ("BOTTOM_OF_PIPE", "BOTTOM_OF_PIPE", "", 0x00002000),
        ("HOST", "HOST", "", 0x00004000),
        ("ALL_GRAPHICS", "ALL_GRAPHICS", "", 0x00008000),
        ("ALL_COMMANDS", "ALL_COMMANDS", "", 0x00010000),
    )

    default_value: bpy.props.EnumProperty(
        name="Pipeline Stage Flags",
        description="Pipeline Stage Flags",
        items=enum_values,
        default="TOP_OF_PIPE",
    )

    def get_value(self) -> typing.Union[int, str]:
        return self.get('default_value', 1)

    def draw(self, context, layout, node, text):
        if self.is_output or self.is_linked:
            layout.label(text=text)
        else:
            layout.prop(self, "default_value", text=text)

    def draw_color(self, context, node):
        return (0.5, 0.75, 0.75, 1.0)

class RenderPassNodeSocket(bpy.types.NodeSocket):
    
    bl_label = "Render Pass Node Socket"

    def draw(self, context, layout, node, text):
        layout.label(text=text)

    def draw_color(self, context, node):
        return (1.0, 0.75, 0.75, 1.0)

class SubpassDependencyNodeSocket(bpy.types.NodeSocket):
    
    bl_label = "Subpass Dependency Node Socket"

    def draw(self, context, layout, node, text):
        layout.label(text=text)

    def draw_color(self, context, node):
        return (0.0, 0.25, 0.75, 1.0)


# Mix-in class for all custom nodes in this tree type.
# Defines a poll function to enable instantiation.
class MyCustomTreeNode:
    @classmethod
    def poll(cls, ntree):
        return ntree.bl_idname == 'CustomTreeType'


class AccessFlagsNode(bpy.types.Node, MyCustomTreeNode):

    bl_label = 'Access Flags node'

    def init(self, context):
        self.inputs.new("AccessFlagsNodeSocket", "Flags")
        
        self.outputs.new('AccessFlagsNodeSocket', "Flags")


class AttachmentNode(bpy.types.Node, MyCustomTreeNode):

    bl_label = 'Attachment node'

    def init(self, context):
        self.inputs.new("FormatNodeSocket", "Format")
        self.inputs.new("SampleCountNodeSocket", "Sample Count")
        self.inputs.new("LoadOperationNodeSocket", "Load Op")
        self.inputs.new("StoreOperationNodeSocket", "Store Op")
        self.inputs.new("LoadOperationNodeSocket", "Stencil Load Op")
        self.inputs.new("StoreOperationNodeSocket", "Stencil Store Op")
        self.inputs.new("ImageLayoutNodeSocket", "Initial Layout")
        self.inputs.new("ImageLayoutNodeSocket", "Final Layout")

        self.outputs.new('AttachmentNodeSocket', "Attachment")


class AttachmentReferenceNode(bpy.types.Node, MyCustomTreeNode):

    bl_label = 'Attachment Reference node'

    def init(self, context):
        self.inputs.new("AttachmentNodeSocket", "Attachment")
        self.inputs["Attachment"].link_limit = 1
        self.inputs.new("ImageLayoutNodeSocket", "Layout")
        self.inputs["Layout"].link_limit = 1

        self.outputs.new('AttachmentReferenceNodeSocket', "Attachment Reference")


# Derived from the Node base type.
class RenderPassNode(bpy.types.Node, MyCustomTreeNode):
    
    bl_idname = 'RenderPassNode'
    bl_label = "Render pass node"
    bl_icon = 'SOUND'

    # === Custom Properties ===
    # These work just like custom properties in ID data blocks
    # Extensive information can be found under
    # http://wiki.blender.org/index.php/Doc:2.6/Manual/Extensions/Python/Properties
    my_string_prop: bpy.props.StringProperty()
    my_float_prop: bpy.props.FloatProperty(default=3.1415926)

    # === Optional Functions ===
    # Initialization function, called when a new node is created.
    # This is the most common place to create the sockets for a node, as shown below.
    # NOTE: this is not the same as the standard __init__ function in Python, which is
    #       a purely internal Python method and unknown to the node system!
    def init(self, context):
        self.inputs.new("NodeSocketString", "Name")
        self.inputs.new('AttachmentNodeSocket', "Attachments")
        self.inputs["Attachments"].link_limit = 0
        self.inputs.new('SubpassNodeSocket', "Subpasses")
        self.inputs["Subpasses"].link_limit = 0
        self.inputs.new('SubpassDependencyNodeSocket', "Dependencies")
        self.inputs["Dependencies"].link_limit = 0

        self.outputs.new('RenderPassNodeSocket', "Render Pass")

class SubpassDependencyNode(bpy.types.Node, MyCustomTreeNode):

    bl_label = 'Subpass Dependency node'

    def init(self, context):
        self.inputs.new("SubpassNodeSocket", "Source Subpass")
        self.inputs.new("SubpassNodeSocket", "Destination Subpass")
        self.inputs.new("PipelineStageFlagsNodeSocket", "Source Stage Mask")
        self.inputs.new("PipelineStageFlagsNodeSocket", "Destination Stage Mask")
        self.inputs["Destination Stage Mask"].default_value = "BOTTOM_OF_PIPE"
        self.inputs.new("AccessFlagsNodeSocket", "Source Access Mask")
        self.inputs.new("AccessFlagsNodeSocket", "Destination Access Mask")
        self.inputs.new("DependencyFlagsNodeSocket", "Dependency Flags")

        self.outputs.new('SubpassDependencyNodeSocket', "Subpass Dependency")

class SubpassNode(bpy.types.Node, MyCustomTreeNode):

    bl_label = 'Subpass node'

    def init(self, context):
        self.inputs.new("PipelineBindPointNodeSocket", "Pipeline Bind Point")
        self.inputs["Pipeline Bind Point"].link_limit = 1
        self.inputs.new("AttachmentReferenceNodeSocket", "Input Attachments")
        self.inputs.new("AttachmentReferenceNodeSocket", "Color Attachments")
        self.inputs.new("AttachmentReferenceNodeSocket", "Resolve Attachments")
        self.inputs.new("AttachmentReferenceNodeSocket", "Depth Stencil Attachment")
        self.inputs["Depth Stencil Attachment"].link_limit = 1
        self.inputs.new("AttachmentNodeSocket", "Preserve Attachments")

        self.outputs.new('SubpassNodeSocket', "Subpass")


import nodeitems_utils

class RenderPassNodeCategory(nodeitems_utils.NodeCategory):
    
    @classmethod
    def poll(cls, context):
        return context.space_data.tree_type == 'CustomTreeType'


render_pass_node_categories = [
    RenderPassNodeCategory('RENDER_PASS', "Render pass", items=[
        nodeitems_utils.NodeItem("AccessFlagsNode"),
        nodeitems_utils.NodeItem("AttachmentNode"),
        nodeitems_utils.NodeItem("AttachmentReferenceNode"),
        nodeitems_utils.NodeItem("RenderPassNode"),
        nodeitems_utils.NodeItem("SubpassDependencyNode"),
        nodeitems_utils.NodeItem("SubpassNode"),
    ]),
]

import functools
import operator
import typing

JSONType = typing.Union[str, int, float, bool, None, typing.Dict[str, typing.Any], typing.List[typing.Any]]

def get_attachment_input(attachment_node: AttachmentNode, input_name: str) -> typing.Any:

    assert len(attachment_node.inputs[input_name].links) == 0

    return attachment_node.inputs[input_name].get_value()


def attachment_to_json(attachment_node: AttachmentNode) -> JSONType:
    
    return {
        "flags": 0, # TODO
        "format": get_attachment_input(attachment_node, "Format"),
        "samples": get_attachment_input(attachment_node, "Sample Count"),
        "load_operation": get_attachment_input(attachment_node, "Load Op"),
        "store_operation": get_attachment_input(attachment_node, "Store Op"),
        "stencil_load_operation": get_attachment_input(attachment_node, "Stencil Load Op"),
        "stencil_store_operation": get_attachment_input(attachment_node, "Stencil Store Op"),
        "initial_layout": get_attachment_input(attachment_node, "Initial Layout"),
        "final_layout": get_attachment_input(attachment_node, "Final Layout"),
    }


def get_attachment_reference_input(attachment_reference_node: AttachmentReferenceNode, input_name: str) -> typing.Any:
    
    assert len(attachment_reference_node.inputs[input_name].links) == 0

    return attachment_reference_node.inputs[input_name].get_value()

def get_subpass_input(subpass_node: SubpassNode, input_name: str) -> typing.Any:
    
    assert len(subpass_node.inputs[input_name].links) == 0

    return subpass_node.inputs[input_name].get_value()

def get_attachment_references(subpass_node: SubpassNode, input_name: str, attachments: typing.List[AttachmentNode]) -> JSONType:

    attachment_reference_nodes = [link.from_node
                                  for link in subpass_node.inputs[input_name].links]

    assert functools.reduce(operator.and_, [node.bl_idname == "AttachmentReferenceNode" and len(node.inputs["Attachment"].links) == 1
                                            for node in attachment_reference_nodes], True)

    attachment_indices = [attachments.index(node.inputs["Attachment"].links[0].from_node)
                          for node in attachment_reference_nodes]

    layouts = [get_attachment_reference_input(node, "Layout")
               for node in attachment_reference_nodes]

    return [{"attachment": attachment_index, "layout": layout}
            for (attachment_index, layout) in zip(attachment_indices, layouts)]

def subpass_to_json(subpass_node: SubpassNode, attachments: typing.List[AttachmentNode]) -> JSONType:

    # TODO implement
    assert len(subpass_node.inputs["Resolve Attachments"].links) == 0

    return {
        "pipeline_bind_point": get_subpass_input(subpass_node, "Pipeline Bind Point"),
        "input_attachments": get_attachment_references(subpass_node, "Input Attachments", attachments),
        "color_attachments": get_attachment_references(subpass_node, "Color Attachments", attachments),
        "resolve_attachments": [],
        "depth_stencil_attachment": get_attachment_references(subpass_node, "Depth Stencil Attachment", attachments),
        "preserve_attachments": [attachments.index(link.from_node)
                                 for link in subpass_node.inputs["Preserve Attachments"].links]
    }

def get_subpass_dependency_input(subpass_dependency_node: SubpassDependencyNode, input_name: str) -> typing.Any:
    
    assert len(subpass_dependency_node.inputs[input_name].links) == 0

    return subpass_dependency_node.inputs[input_name].get_value()


def get_subpass_index(subpass_socket: SubpassNodeSocket, subpasses: typing.List[SubpassNode]) -> typing.Union[int, str]:

    assert len(subpass_socket.links) <= 1

    if len(subpass_socket.links) == 0:
        return "external"

    else:
        return subpasses.index(subpass_socket.links[0].from_node) 


def subpass_dependency_to_json(subpass_depency_node: SubpassDependencyNode, subpasses: typing.List[SubpassNode]) -> JSONType:
    
    return {
        "source_subpass": get_subpass_index(subpass_depency_node.inputs["Source Subpass"], subpasses),
        "destination_subpass": get_subpass_index(subpass_depency_node.inputs["Destination Subpass"], subpasses),
        "source_stage_mask": get_subpass_dependency_input(subpass_depency_node, "Source Stage Mask"),
        "destination_stage_mask": get_subpass_dependency_input(subpass_depency_node, "Destination Stage Mask"),
        "source_access_mask": get_subpass_dependency_input(subpass_depency_node, "Source Access Mask"),
        "destination_access_mask": get_subpass_dependency_input(subpass_depency_node, "Destination Access Mask"),
        "dependency_flags": get_subpass_dependency_input(subpass_depency_node, "Dependency Flags")
    }

def render_pass_to_json(nodes: typing.List[bpy.types.Node]) -> JSONType:
    
    render_pass_nodes = [node
                         for node in nodes
                         if node.bl_idname == "RenderPassNode"]

    render_pass_names = [node.inputs["Name"].default_value
                         for node in render_pass_nodes]

    assert functools.reduce(operator.and_, [len(node.inputs["Subpasses"].links) > 0
                                            for node in render_pass_nodes], True)

    attachments_per_render_pass = [[link.from_node 
                                    for link in node.inputs["Attachments"].links]
                                   for node in render_pass_nodes]

    assert functools.reduce(operator.and_, [attachment.bl_idname == "AttachmentNode"
                                            for attachments in attachments_per_render_pass
                                            for attachment in attachments], True)

    subpasses_per_render_pass = [[link.from_node 
                                  for link in node.inputs["Subpasses"].links]
                                 for node in render_pass_nodes]

    assert functools.reduce(operator.and_, [subpass.bl_idname == "SubpassNode"
                                            for subpasses in subpasses_per_render_pass
                                            for subpass in subpasses], True)

    subpass_dependencies_per_render_pass = [[link.from_node 
                                             for link in node.inputs["Dependencies"].links]
                                            for node in render_pass_nodes]

    assert functools.reduce(operator.and_, [subpass_dependency.bl_idname == "SubpassDependencyNode"
                                            for subpass_dependencies in subpass_dependencies_per_render_pass
                                            for subpass_dependency in subpass_dependencies], True)

    attachments_per_render_pass_jsons = [[attachment_to_json(attachment) for attachment in attachments]
                                         for attachments in attachments_per_render_pass]

    subpasses_per_render_pass_jsons = [[subpass_to_json(subpass, attachments_per_render_pass[render_pass_index]) for subpass in subpasses]
                                       for render_pass_index, subpasses in enumerate(subpasses_per_render_pass)]

    dependencies_per_render_pass_jsons = [[subpass_dependency_to_json(subpass_dependency, subpasses_per_render_pass[render_pass_index]) for subpass_dependency in subpass_dependencies]
                                          for render_pass_index, subpass_dependencies in enumerate(subpass_dependencies_per_render_pass)]

    return [{"name": name, "attachments": attachment_jsons, "subpasses": subpass_jsons, "dependencies": dependency_jsons}
            for (name, attachment_jsons, subpass_jsons, dependency_jsons) in zip(render_pass_names, attachments_per_render_pass_jsons, subpasses_per_render_pass_jsons, dependencies_per_render_pass_jsons)]
