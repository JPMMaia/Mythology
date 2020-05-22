import bpy
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

    enmu_values = (
        ("NONE", "NONE", ""),
        ("INDIRECT_COMMAND_READ", "INDIRECT_COMMAND_READ", ""),
        ("INDEX_READ", "INDEX_READ", ""),
        ("VERTEX_ATTRIBUTE_READ", "VERTEX_ATTRIBUTE_READ", ""),
        ("UNIFORM_READ", "UNIFORM_READ", ""),
        ("INPUT_ATTACHMENT_READ", "INPUT_ATTACHMENT_READ", ""),
        ("SHADER_READ", "SHADER_READ", ""),
        ("SHADER_WRITE", "SHADER_WRITE", ""),
        ("COLOR_ATTACHMENT_READ", "COLOR_ATTACHMENT_READ", ""),
        ("COLOR_ATTACHMENT_WRITE", "COLOR_ATTACHMENT_WRITE", ""),
        ("DEPTH_STENCIL_ATTACHMENT_READ", "DEPTH_STENCIL_ATTACHMENT_READ", ""),
        ("DEPTH_STENCIL_ATTACHMENT_WRITE", "DEPTH_STENCIL_ATTACHMENT_WRITE", ""),
        ("TRANSFER_READ", "TRANSFER_READ", ""),
        ("TRANSFER_WRITE", "TRANSFER_WRITE", ""),
        ("HOST_READ", "HOST_READ", ""),
        ("HOST_WRITE", "HOST_WRITE", ""),
        ("MEMORY_READ", "MEMORY_READ", ""),
        ("MEMORY_WRITE", "MEMORY_WRITE", ""),
    )

    default_value: bpy.props.EnumProperty(
        name="Access Flags",
        description="Access Flags",
        items=enum_values,
        default="NONE",
    )

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

    enmu_values = (
        ("NONE", "NONE", ""),
        ("BY_REGION", "BY_REGION", ""),
        ("DEVICE_GROUP", "DEVICE_GROUP", ""),
        ("VIEW_LOCAL", "VIEW_LOCAL", ""),
    )

    default_value: bpy.props.EnumProperty(
        name="Dependency Flags",
        description="Dependency Flags",
        items=enum_values,
        default="NONE",
    )

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
        ("UNDEFINED", "UNDEFINED", ""),
        ("R8_UNORM", "R8_UNORM", ""),
        ("R8_SNORM", "R8_SNORM", ""),
        ("R8_USCALED", "R8_USCALED", ""),
        ("R8_SSCALED", "R8_SSCALED", ""),
        ("R8_UINT", "R8_UINT", ""),
        ("R8_SINT", "R8_SINT", ""),
        ("R8_SRGB", "R8_SRGB", ""),
        ("R8G8_UNORM", "R8G8_UNORM", ""),
        ("R8G8_SNORM", "R8G8_SNORM", ""),
        ("R8G8_USCALED", "R8G8_USCALED", ""),
        ("R8G8_SSCALED", "R8G8_SSCALED", ""),
        ("R8G8_UINT", "R8G8_UINT", ""),
        ("R8G8_SINT", "R8G8_SINT", ""),
        ("R8G8_SRGB", "R8G8_SRGB", ""),
        ("R8G8B8_UNORM", "R8G8B8_UNORM", ""),
        ("R8G8B8_SNORM", "R8G8B8_SNORM", ""),
        ("R8G8B8_USCALED", "R8G8B8_USCALED", ""),
        ("R8G8B8_SSCALED", "R8G8B8_SSCALED", ""),
        ("R8G8B8_UINT", "R8G8B8_UINT", ""),
        ("R8G8B8_SINT", "R8G8B8_SINT", ""),
        ("R8G8B8_SRGB", "R8G8B8_SRGB", ""),
        ("B8G8R8_UNORM", "B8G8R8_UNORM", ""),
        ("B8G8R8_SNORM", "B8G8R8_SNORM", ""),
        ("B8G8R8_USCALED", "B8G8R8_USCALED", ""),
        ("B8G8R8_SSCALED", "B8G8R8_SSCALED", ""),
        ("B8G8R8_UINT", "B8G8R8_UINT", ""),
        ("B8G8R8_SINT", "B8G8R8_SINT", ""),
        ("B8G8R8_SRGB", "B8G8R8_SRGB", ""),
        ("R8G8B8A8_UNORM", "R8G8B8A8_UNORM", ""),
        ("R8G8B8A8_SNORM", "R8G8B8A8_SNORM", ""),
        ("R8G8B8A8_USCALED", "R8G8B8A8_USCALED", ""),
        ("R8G8B8A8_SSCALED", "R8G8B8A8_SSCALED", ""),
        ("R8G8B8A8_UINT", "R8G8B8A8_UINT", ""),
        ("R8G8B8A8_SINT", "R8G8B8A8_SINT", ""),
        ("R8G8B8A8_SRGB", "R8G8B8A8_SRGB", ""),
        ("B8G8R8A8_UNORM", "B8G8R8A8_UNORM", ""),
        ("B8G8R8A8_SNORM", "B8G8R8A8_SNORM", ""),
        ("B8G8R8A8_USCALED", "B8G8R8A8_USCALED", ""),
        ("B8G8R8A8_SSCALED", "B8G8R8A8_SSCALED", ""),
        ("B8G8R8A8_UINT", "B8G8R8A8_UINT", ""),
        ("B8G8R8A8_SINT", "B8G8R8A8_SINT", ""),
        ("B8G8R8A8_SRGB", "B8G8R8A8_SRGB", ""),
        ("R16_UNORM", "R16_UNORM", ""),
        ("R16_SNORM", "R16_SNORM", ""),
        ("R16_USCALED", "R16_USCALED", ""),
        ("R16_SSCALED", "R16_SSCALED", ""),
        ("R16_UINT", "R16_UINT", ""),
        ("R16_SINT", "R16_SINT", ""),
        ("R16_SFLOAT", "R16_SFLOAT", ""),
        ("R16G16_UNORM", "R16G16_UNORM", ""),
        ("R16G16_SNORM", "R16G16_SNORM", ""),
        ("R16G16_USCALED", "R16G16_USCALED", ""),
        ("R16G16_SSCALED", "R16G16_SSCALED", ""),
        ("R16G16_UINT", "R16G16_UINT", ""),
        ("R16G16_SINT", "R16G16_SINT", ""),
        ("R16G16_SFLOAT", "R16G16_SFLOAT", ""),
        ("R16G16B16_UNORM", "R16G16B16_UNORM", ""),
        ("R16G16B16_SNORM", "R16G16B16_SNORM", ""),
        ("R16G16B16_USCALED", "R16G16B16_USCALED", ""),
        ("R16G16B16_SSCALED", "R16G16B16_SSCALED", ""),
        ("R16G16B16_UINT", "R16G16B16_UINT", ""),
        ("R16G16B16_SINT", "R16G16B16_SINT", ""),
        ("R16G16B16_SFLOAT", "R16G16B16_SFLOAT", ""),
        ("R16G16B16A16_UNORM", "R16G16B16A16_UNORM", ""),
        ("R16G16B16A16_SNORM", "R16G16B16A16_SNORM", ""),
        ("R16G16B16A16_USCALED", "R16G16B16A16_USCALED", ""),
        ("R16G16B16A16_SSCALED", "R16G16B16A16_SSCALED", ""),
        ("R16G16B16A16_UINT", "R16G16B16A16_UINT", ""),
        ("R16G16B16A16_SINT", "R16G16B16A16_SINT", ""),
        ("R16G16B16A16_SFLOAT", "R16G16B16A16_SFLOAT", ""),
        ("R32_UINT", "R32_UINT", ""),
        ("R32_SINT", "R32_SINT", ""),
        ("R32_SFLOAT", "R32_SFLOAT", ""),
        ("R32G32_UINT", "R32G32_UINT", ""),
        ("R32G32_SINT", "R32G32_SINT", ""),
        ("R32G32_SFLOAT", "R32G32_SFLOAT", ""),
        ("R32G32B32_UINT", "R32G32B32_UINT", ""),
        ("R32G32B32_SINT", "R32G32B32_SINT", ""),
        ("R32G32B32_SFLOAT", "R32G32B32_SFLOAT", ""),
        ("R32G32B32A32_UINT", "R32G32B32A32_UINT", ""),
        ("R32G32B32A32_SINT", "R32G32B32A32_SINT", ""),
        ("R32G32B32A32_SFLOAT", "R32G32B32A32_SFLOAT", ""),
        ("R64_UINT", "R64_UINT", ""),
        ("R64_SINT", "R64_SINT", ""),
        ("R64_SFLOAT", "R64_SFLOAT", ""),
        ("R64G64_UINT", "R64G64_UINT", ""),
        ("R64G64_SINT", "R64G64_SINT", ""),
        ("R64G64_SFLOAT", "R64G64_SFLOAT", ""),
        ("R64G64B64_UINT", "R64G64B64_UINT", ""),
        ("R64G64B64_SINT", "R64G64B64_SINT", ""),
        ("R64G64B64_SFLOAT", "R64G64B64_SFLOAT", ""),
        ("R64G64B64A64_UINT", "R64G64B64A64_UINT", ""),
        ("R64G64B64A64_SINT", "R64G64B64A64_SINT", ""),
        ("R64G64B64A64_SFLOAT", "R64G64B64A64_SFLOAT", ""),
        ("B10G11R11_UFLOAT_PACK32", "B10G11R11_UFLOAT_PACK32", ""),
        ("E5B9G9R9_UFLOAT_PACK32", "E5B9G9R9_UFLOAT_PACK32", ""),
        ("D16_UNORM", "D16_UNORM", ""),
        ("X8_D24_UNORM_PACK32", "X8_D24_UNORM_PACK32", ""),
        ("D32_SFLOAT", "D32_SFLOAT", ""),
        ("S8_UINT", "S8_UINT", ""),
        ("D16_UNORM_S8_UINT", "D16_UNORM_S8_UINT", ""),
        ("D24_UNORM_S8_UINT", "D24_UNORM_S8_UINT", ""),
        ("D32_SFLOAT_S8_UINT", "D32_SFLOAT_S8_UINT", ""),
    )

    default_value: bpy.props.EnumProperty(
        name="Format",
        description="Format",
        items=enum_values,
        default='UNDEFINED',
    )

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
        ("1", "1", ""),
        ("2", "2", ""),
        ("4", "4", ""),
        ("8", "8", ""),
        ("16", "16", ""),
        ("32", "32", ""),
        ("64", "64", ""),
    )

    default_value: bpy.props.EnumProperty(
        name="Sample count",
        description="Sample count",
        items=enum_values,
        default="1",
    )

    def draw(self, context, layout, node, text):
        if self.is_output or self.is_linked:
            layout.label(text=text)
        else:
            layout.prop(self, "default_value", text=text)

    def draw_color(self, context, node):
        return (0.0, 0.5, 1.0, 1.0)

class LoadOperationNodeSocket(bpy.types.NodeSocket):
    
    bl_label = "Load operation Socket"

    enmu_values = (
        ("LOAD", "Load", ""),
        ("CLEAR", "Clear", ""),
        ("DONT_CARE", "Don't care", ""),
    )

    default_value: bpy.props.EnumProperty(
        name="Load operation",
        description="Load operation",
        items=enum_values,
        default="LOAD",
    )

    def draw(self, context, layout, node, text):
        if self.is_output or self.is_linked:
            layout.label(text=text)
        else:
            layout.prop(self, "default_value", text=text)

    def draw_color(self, context, node):
        return (1.0, 0.5, 0.5, 1.0)


class StoreOperationNodeSocket(bpy.types.NodeSocket):
    
    bl_label = "Store operation Socket"

    enmu_values = (
        ("STORE", "Store", ""),
        ("DONT_CARE", "Don't care", ""),
    )

    default_value: bpy.props.EnumProperty(
        name="Store operation",
        description="Store operation",
        items=enum_values,
        default="STORE",
    )

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

    enmu_values = (
        ("UNDEFINED", "UNDEFINED", ""),
        ("GENERAL", "GENERAL", ""),
        ("COLOR_ATTACHMENT_OPTIMAL", "COLOR_ATTACHMENT_OPTIMAL", ""),
        ("DEPTH_STENCIL_ATTACHMENT_OPTIMAL", "DEPTH_STENCIL_ATTACHMENT_OPTIMAL", ""),
        ("DEPTH_STENCIL_READ_ONLY_OPTIMAL", "DEPTH_STENCIL_READ_ONLY_OPTIMAL", ""),
        ("SHADER_READ_ONLY_OPTIMAL", "SHADER_READ_ONLY_OPTIMAL", ""),
        ("TRANSFER_SRC_OPTIMAL", "TRANSFER_SRC_OPTIMAL", ""),
        ("TRANSFER_DST_OPTIMAL", "TRANSFER_DST_OPTIMAL", ""),
        ("PREINITIALIZED", "PREINITIALIZED", ""),
        ("DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL", "DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL", ""),
        ("DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL", "DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL", ""),
        ("DEPTH_ATTACHMENT_OPTIMAL", "DEPTH_ATTACHMENT_OPTIMAL", ""),
        ("DEPTH_READ_ONLY_OPTIMAL", "DEPTH_READ_ONLY_OPTIMAL", ""),
        ("STENCIL_ATTACHMENT_OPTIMAL", "STENCIL_ATTACHMENT_OPTIMAL", ""),
        ("STENCIL_READ_ONLY_OPTIMAL", "STENCIL_READ_ONLY_OPTIMAL", ""),
    )

    default_value: bpy.props.EnumProperty(
        name="Image layout",
        description="Image layout",
        items=enum_values,
        default="UNDEFINED",
    )

    def draw(self, context, layout, node, text):
        if self.is_output or self.is_linked:
            layout.label(text=text)
        else:
            layout.prop(self, "default_value", text=text)

    def draw_color(self, context, node):
        return (0.75, 0.25, 0.5, 1.0)


class PipelineBindPointNodeSocket(bpy.types.NodeSocket):
    
    bl_label = "Pipeline Bind Point Node Socket"

    enmu_values = (
        ("GRAPHICS", "Graphics", ""),
        ("COMPUTE", "Compute", ""),
    )

    default_value: bpy.props.EnumProperty(
        name="Pipeline Bind Point",
        description="Pipeline Bind Point",
        items=enum_values,
        default="GRAPHICS",
    )

    def draw(self, context, layout, node, text):
        if self.is_output or self.is_linked:
            layout.label(text=text)
        else:
            layout.prop(self, "default_value", text=text)

    def draw_color(self, context, node):
        return (0.25, 0.5, 0.75, 1.0)


class PipelineStageFlagsNodeSocket(bpy.types.NodeSocket):
    
    bl_label = "Pipeline Stage Flags Node Socket"

    enmu_values = (
        ("TOP_OF_PIPE", "TOP_OF_PIPE", ""),
        ("DRAW_INDIRECT", "DRAW_INDIRECT", ""),
        ("VERTEX_INPUT", "VERTEX_INPUT", ""),
        ("VERTEX_SHADER", "VERTEX_SHADER", ""),
        ("TESSELLATION_CONTROL_SHADER", "TESSELLATION_CONTROL_SHADER", ""),
        ("TESSELLATION_EVALUATION_SHADER", "TESSELLATION_EVALUATION_SHADER", ""),
        ("GEOMETRY_SHADER", "GEOMETRY_SHADER", ""),
        ("FRAGMENT_SHADER", "FRAGMENT_SHADER", ""),
        ("EARLY_FRAGMENT_TESTS", "EARLY_FRAGMENT_TESTS", ""),
        ("LATE_FRAGMENT_TESTS", "LATE_FRAGMENT_TESTS", ""),
        ("COLOR_ATTACHMENT_OUTPUT", "COLOR_ATTACHMENT_OUTPUT", ""),
        ("COMPUTE_SHADER", "COMPUTE_SHADER", ""),
        ("TRANSFER", "TRANSFER", ""),
        ("BOTTOM_OF_PIPE", "BOTTOM_OF_PIPE", ""),
        ("HOST", "HOST", ""),
        ("ALL_GRAPHICS", "ALL_GRAPHICS", ""),
        ("ALL_COMMANDS", "ALL_COMMANDS", ""),
    )

    default_value: bpy.props.EnumProperty(
        name="Pipeline Stage Flags",
        description="Pipeline Stage Flags",
        items=enum_values,
        default="TOP_OF_PIPE",
    )

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
        self.inputs.new("ImageLayoutNodeSocket", "Layout")

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
        self.inputs.new('AttachmentNodeSocket', "Attachments")
        self.inputs.new('SubpassNodeSocket', "Subpasses")
        self.inputs.new('SubpassDependencyNodeSocket', "Dependencies")

        self.outputs.new('RenderPassNodeSocket', "Render Pass")

class SubpassDependencyNode(bpy.types.Node, MyCustomTreeNode):

    bl_label = 'Subpass Dependency node'

    def init(self, context):
        self.inputs.new("SubpassNodeSocket", "Source Subpass")
        self.inputs.new("SubpassNodeSocket", "Destination Subpass")
        self.inputs.new("PipelineStageFlagsNodeSocket", "Source Stage Mask")
        self.inputs.new("PipelineStageFlagsNodeSocket", "Destination Stage Mask")
        self.inputs.new("AccessFlagsNodeSocket", "Source Access Mask")
        self.inputs.new("AccessFlagsNodeSocket", "Destination Access Mask")
        self.inputs.new("DependencyFlagsNodeSocket", "Dependency Flags")

        self.outputs.new('SubpassDependencyNodeSocket', "Subpass Dependency")

class SubpassNode(bpy.types.Node, MyCustomTreeNode):

    bl_label = 'Subpass node'

    def init(self, context):
        self.inputs.new("PipelineBindPointNodeSocket", "Pipeline Bind Point")
        self.inputs.new("AttachmentReferenceNodeSocket", "Input Attachments")
        self.inputs.new("AttachmentReferenceNodeSocket", "Color Attachments")
        self.inputs.new("AttachmentReferenceNodeSocket", "Resolve Attachments")
        self.inputs.new("AttachmentReferenceNodeSocket", "Depth Stencil Attachment")
        self.inputs.new("AttachmentNodeSocket", "Preserve Attachments")

        self.outputs.new('SubpassNodeSocket', "Subpass")


import nodeitems_utils

class RenderPassNodeCategory(nodeitems_utils.NodeCategory):
    
    @classmethod
    def poll(cls, context):
        return context.space_data.tree_type == 'CustomTreeType'


render_pass_node_categories = [
    RenderPassNodeCategory('RENDER_PASS', "Render pass", items=[
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

    assert len(attachment_node.inputs[input_name]) == 0

    return attachment_node.inputs[input_name].default_value


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


def get_subpass_input(subpass_node: SubpassNode, input_name: str) -> typing.Any:
    
    assert len(subpass_node.inputs[input_name]) == 0

    return subpass_node.inputs[input_name].default_value

def subpass_to_json(subpass_node: SubpassNode, attachments: typing.List[AttachmentNode]) -> JSONType:
    
    return {
        "pipeline_bind_point": get_subpass_input(subpass_node, "Pipeline Bind Point"),
        "input_attachments": [],
    }

def subpass_dependency_to_json(subpass_depency_node: SubpassDependencyNode) -> JSONType:
    return {}

def render_pass_to_json(nodes: typing.List[bpy.types.Node]) -> JSONType:
    
    render_pass_nodes = [node
                         for node in nodes
                         if node.bl_idname == "RenderPassNode"]

    assert functools.reduce(operator.and_, [len(node.inputs["Subpasses"].links) > 0
                                            for node in render_pass_nodes])

    attachments_per_render_pass = [[link.from_node 
                                    for link in node.inputs["Attachments"].links]
                                   for node in render_pass_nodes]

    assert functools.reduce(operator.and_, [attachment.bl_idname == "AttachmentNode"
                                            for attachments in attachments_per_render_pass
                                            for attachment in attachments])

    subpasses_per_render_pass = [[link.from_node 
                                  for link in node.inputs["Subpasses"].links]
                                 for node in render_pass_nodes]

    assert functools.reduce(operator.and_, [subpass.bl_idname == "SubpassNode"
                                            for subpasses in subpasses_per_render_pass
                                            for subpass in subpasses])

    attachments_per_render_pass_jsons = [[attachment_to_json(attachment) for attachment in attachments]
                                         for attachments in attachments_per_render_pass]

    subpasses_per_render_pass_jsons = [[subpass_to_json(subpass, subpasses_per_render_pass[render_pass_index]) for subpass in subpasses]
                                       for render_pass_index, subpasses in enumerate(subpasses_per_render_pass)]

    return {"hello": 1}
