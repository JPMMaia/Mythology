import bpy
import typing

from bpy.types import NodeTree, Node, NodeSocket

from .common import get_enum_property_int_value
from .render_node_tree import RenderTreeNode
from .vulkan_enums import *


class AccessFlagsNodeSocket(bpy.types.NodeSocket):

    bl_label = "Access Flags Socket"

    enum_values = access_flag_values

    default_value: bpy.props.EnumProperty(
        name="Access Flags",
        description="Access Flags",
        items=enum_values,
        options={"ANIMATABLE", "ENUM_FLAG"},
    )

    def get_value(self) -> typing.Union[int, str]:
        return get_enum_property_int_value(self.enum_values, self.default_value, 0)

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

    enum_values = dependency_flag_values

    default_value: bpy.props.EnumProperty(
        name="Dependency Flags",
        description="Dependency Flags",
        items=enum_values,
        options={"ANIMATABLE", "ENUM_FLAG"},
    )

    def get_value(self) -> typing.Union[int, str]:
        return get_enum_property_int_value(self.enum_values, self.default_value, 0)

    def draw(self, context, layout, node, text):
        if self.is_output or self.is_linked:
            layout.label(text=text)
        else:
            layout.prop(self, "default_value", text=text)

    def draw_color(self, context, node):
        return (1.0, 0.5, 0.0, 1.0)


class FormatNodeSocket(bpy.types.NodeSocket):

    bl_label = "Format Node Socket"

    default_value: bpy.props.EnumProperty(
        name="Format",
        description="Format",
        items=format_values,
        default="UNDEFINED",
    )

    def get_value(self) -> typing.Union[int, str]:
        return get_enum_property_int_value(format_values, self.default_value, 0)

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
        return get_enum_property_int_value(self.enum_values, self.default_value, 1)

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
        return get_enum_property_int_value(self.enum_values, self.default_value, 2)

    def draw(self, context, layout, node, text):
        if self.is_output or self.is_linked:
            layout.label(text=text)
        else:
            layout.prop(self, "default_value", text=text)

    def draw_color(self, context, node):
        return (1.0, 0.5, 0.5, 1.0)


class StoreOperationNodeSocket(bpy.types.NodeSocket):

    bl_label = "Store operation Socket"

    enum_values = [
        ("STORE", "Store", "", 0),
        ("DONT_CARE", "Don't care", "", 1),
    ]

    default_value: bpy.props.EnumProperty(
        name="Store operation",
        description="Store operation",
        items=enum_values,
        default="STORE",
    )

    def get_value(self) -> typing.Union[int, str]:
        return get_enum_property_int_value(self.enum_values, self.default_value, 0)

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

    enum_values = image_layout_values

    default_value: bpy.props.EnumProperty(
        name="Image layout",
        description="Image layout",
        items=enum_values,
        default="UNDEFINED",
    )

    def get_value(self) -> int:
        return image_layout_values_to_int[self.default_value]

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
        return get_enum_property_int_value(self.enum_values, self.default_value, 0)

    def draw(self, context, layout, node, text):
        if self.is_output or self.is_linked:
            layout.label(text=text)
        else:
            layout.prop(self, "default_value", text=text)

    def draw_color(self, context, node):
        return (0.25, 0.5, 0.75, 1.0)


class PipelineStageFlagsNodeSocket(bpy.types.NodeSocket):

    bl_label = "Pipeline Stage Flags Node Socket"

    enum_values = pipeline_stage_flag_values

    default_value: bpy.props.EnumProperty(
        name="Pipeline Stage Flags",
        description="Pipeline Stage Flags",
        items=enum_values,
        default="TOP_OF_PIPE",
    )

    def get_value(self) -> typing.Union[int, str]:
        return get_enum_property_int_value(self.enum_values, self.default_value, 1)

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


class AccessFlagsNode(bpy.types.Node, RenderTreeNode):

    bl_label = "Access Flags node"

    def init(self, context):
        self.inputs.new("AccessFlagsNodeSocket", "Flags")

        self.outputs.new("AccessFlagsNodeSocket", "Flags")


class AttachmentNode(bpy.types.Node, RenderTreeNode):

    bl_label = "Attachment node"

    def init(self, context):
        self.inputs.new("FormatNodeSocket", "Format")
        self.inputs.new("SampleCountNodeSocket", "Sample Count")
        self.inputs.new("LoadOperationNodeSocket", "Load Op")
        self.inputs.new("StoreOperationNodeSocket", "Store Op")
        self.inputs.new("LoadOperationNodeSocket", "Stencil Load Op")
        self.inputs.new("StoreOperationNodeSocket", "Stencil Store Op")
        self.inputs.new("ImageLayoutNodeSocket", "Initial Layout")
        self.inputs.new("ImageLayoutNodeSocket", "Final Layout")

        self.outputs.new("AttachmentNodeSocket", "Attachment")


class AttachmentReferenceNode(bpy.types.Node, RenderTreeNode):

    bl_label = "Attachment Reference node"

    def init(self, context):
        self.inputs.new("AttachmentNodeSocket", "Attachment")
        self.inputs["Attachment"].link_limit = 1
        self.inputs.new("ImageLayoutNodeSocket", "Layout")
        self.inputs["Layout"].link_limit = 1

        self.outputs.new("AttachmentReferenceNodeSocket", "Attachment Reference")


# Derived from the Node base type.
class RenderPassNode(bpy.types.Node, RenderTreeNode):

    bl_idname = "RenderPassNode"
    bl_label = "Render pass node"
    bl_icon = "SOUND"

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
        self.inputs.new("AttachmentNodeSocket", "Attachments")
        self.inputs["Attachments"].link_limit = 0
        self.inputs.new("SubpassNodeSocket", "Subpasses")
        self.inputs["Subpasses"].link_limit = 0
        self.inputs.new("SubpassDependencyNodeSocket", "Dependencies")
        self.inputs["Dependencies"].link_limit = 0

        self.outputs.new("RenderPassNodeSocket", "Render Pass")


class SubpassDependencyNode(bpy.types.Node, RenderTreeNode):

    bl_label = "Subpass Dependency node"

    def init(self, context):
        self.inputs.new("SubpassNodeSocket", "Source Subpass")
        self.inputs.new("SubpassNodeSocket", "Destination Subpass")
        self.inputs.new("PipelineStageFlagsNodeSocket", "Source Stage Mask")
        self.inputs.new("PipelineStageFlagsNodeSocket", "Destination Stage Mask")
        self.inputs["Destination Stage Mask"].default_value = "BOTTOM_OF_PIPE"
        self.inputs.new("AccessFlagsNodeSocket", "Source Access Mask")
        self.inputs.new("AccessFlagsNodeSocket", "Destination Access Mask")
        self.inputs.new("DependencyFlagsNodeSocket", "Dependency Flags")

        self.outputs.new("SubpassDependencyNodeSocket", "Subpass Dependency")


class SubpassNode(bpy.types.Node, RenderTreeNode):

    bl_label = "Subpass node"

    def init(self, context):
        self.inputs.new("PipelineBindPointNodeSocket", "Pipeline Bind Point")
        self.inputs["Pipeline Bind Point"].link_limit = 1
        self.inputs.new("AttachmentReferenceNodeSocket", "Input Attachments")
        self.inputs.new("AttachmentReferenceNodeSocket", "Color Attachments")
        self.inputs.new("AttachmentReferenceNodeSocket", "Resolve Attachments")
        self.inputs.new("AttachmentReferenceNodeSocket", "Depth Stencil Attachment")
        self.inputs["Depth Stencil Attachment"].link_limit = 1
        self.inputs.new("AttachmentNodeSocket", "Preserve Attachments")

        self.outputs.new("SubpassNodeSocket", "Subpass")


import nodeitems_utils


class RenderPassNodeCategory(nodeitems_utils.NodeCategory):
    @classmethod
    def poll(cls, context):
        return context.space_data.tree_type == "RenderNodeTree"


render_pass_node_categories = [
    RenderPassNodeCategory(
        "RENDER_PASS",
        "Render pass",
        items=[
            nodeitems_utils.NodeItem("AccessFlagsNode"),
            nodeitems_utils.NodeItem("AttachmentNode"),
            nodeitems_utils.NodeItem("AttachmentReferenceNode"),
            nodeitems_utils.NodeItem("RenderPassNode"),
            nodeitems_utils.NodeItem("SubpassDependencyNode"),
            nodeitems_utils.NodeItem("SubpassNode"),
        ],
    ),
]

import functools
import operator
import typing

JSONType = typing.Union[
    str, int, float, bool, None, typing.Dict[str, typing.Any], typing.List[typing.Any]
]


def get_attachment_input(
    attachment_node: AttachmentNode, input_name: str
) -> typing.Any:

    assert len(attachment_node.inputs[input_name].links) == 0

    return attachment_node.inputs[input_name].get_value()


def attachment_to_json(attachment_node: AttachmentNode) -> JSONType:

    return {
        "flags": 0,  # TODO
        "format": get_attachment_input(attachment_node, "Format"),
        "samples": get_attachment_input(attachment_node, "Sample Count"),
        "load_operation": get_attachment_input(attachment_node, "Load Op"),
        "store_operation": get_attachment_input(attachment_node, "Store Op"),
        "stencil_load_operation": get_attachment_input(
            attachment_node, "Stencil Load Op"
        ),
        "stencil_store_operation": get_attachment_input(
            attachment_node, "Stencil Store Op"
        ),
        "initial_layout": get_attachment_input(attachment_node, "Initial Layout"),
        "final_layout": get_attachment_input(attachment_node, "Final Layout"),
    }


def get_attachment_reference_input(
    attachment_reference_node: AttachmentReferenceNode, input_name: str
) -> typing.Any:

    assert len(attachment_reference_node.inputs[input_name].links) == 0

    return attachment_reference_node.inputs[input_name].get_value()


def get_subpass_input(subpass_node: SubpassNode, input_name: str) -> typing.Any:

    assert len(subpass_node.inputs[input_name].links) == 0

    return subpass_node.inputs[input_name].get_value()


def get_attachment_references(
    subpass_node: SubpassNode, input_name: str, attachments: typing.List[AttachmentNode]
) -> JSONType:

    attachment_reference_nodes = [
        link.from_node for link in subpass_node.inputs[input_name].links
    ]

    assert functools.reduce(
        operator.and_,
        [
            node.bl_idname == "AttachmentReferenceNode"
            and len(node.inputs["Attachment"].links) == 1
            for node in attachment_reference_nodes
        ],
        True,
    )

    attachment_indices = [
        attachments.index(node.inputs["Attachment"].links[0].from_node)
        for node in attachment_reference_nodes
    ]

    layouts = [
        get_attachment_reference_input(node, "Layout")
        for node in attachment_reference_nodes
    ]

    return [
        {"attachment": attachment_index, "layout": layout}
        for (attachment_index, layout) in zip(attachment_indices, layouts)
    ]


def subpass_to_json(
    subpass_node: SubpassNode, attachments: typing.List[AttachmentNode]
) -> JSONType:

    # TODO implement
    assert len(subpass_node.inputs["Resolve Attachments"].links) == 0

    depth_stencil_attachments = get_attachment_references(
        subpass_node, "Depth Stencil Attachment", attachments
    )
    assert len(depth_stencil_attachments) <= 1
    depth_stencil_attachment = (
        depth_stencil_attachments[0] if len(depth_stencil_attachments) == 1 else {}
    )

    return {
        "pipeline_bind_point": get_subpass_input(subpass_node, "Pipeline Bind Point"),
        "input_attachments": get_attachment_references(
            subpass_node, "Input Attachments", attachments
        ),
        "color_attachments": get_attachment_references(
            subpass_node, "Color Attachments", attachments
        ),
        "resolve_attachments": [],
        "depth_stencil_attachment": depth_stencil_attachment,
        "preserve_attachments": [
            attachments.index(link.from_node)
            for link in subpass_node.inputs["Preserve Attachments"].links
        ],
    }


def get_subpass_dependency_input(
    subpass_dependency_node: SubpassDependencyNode, input_name: str
) -> typing.Any:

    assert len(subpass_dependency_node.inputs[input_name].links) == 0

    return subpass_dependency_node.inputs[input_name].get_value()


def get_subpass_index(
    subpass_socket: SubpassNodeSocket, subpasses: typing.List[SubpassNode]
) -> typing.Union[int, str]:

    assert len(subpass_socket.links) <= 1

    if len(subpass_socket.links) == 0:
        return "external"

    else:
        return subpasses.index(subpass_socket.links[0].from_node)


def subpass_dependency_to_json(
    subpass_depency_node: SubpassDependencyNode, subpasses: typing.List[SubpassNode]
) -> JSONType:

    return {
        "source_subpass": get_subpass_index(
            subpass_depency_node.inputs["Source Subpass"], subpasses
        ),
        "destination_subpass": get_subpass_index(
            subpass_depency_node.inputs["Destination Subpass"], subpasses
        ),
        "source_stage_mask": get_subpass_dependency_input(
            subpass_depency_node, "Source Stage Mask"
        ),
        "destination_stage_mask": get_subpass_dependency_input(
            subpass_depency_node, "Destination Stage Mask"
        ),
        "source_access_mask": get_subpass_dependency_input(
            subpass_depency_node, "Source Access Mask"
        ),
        "destination_access_mask": get_subpass_dependency_input(
            subpass_depency_node, "Destination Access Mask"
        ),
        "dependency_flags": get_subpass_dependency_input(
            subpass_depency_node, "Dependency Flags"
        ),
    }


def render_pass_to_json(
    nodes: typing.List[bpy.types.Node],
) -> typing.Tuple[
    typing.List[RenderPassNode], typing.List[typing.List[SubpassNode]], JSONType
]:

    render_pass_nodes = [node for node in nodes if node.bl_idname == "RenderPassNode"]

    render_pass_names = [
        node.inputs["Name"].default_value for node in render_pass_nodes
    ]

    assert functools.reduce(
        operator.and_,
        [len(node.inputs["Subpasses"].links) > 0 for node in render_pass_nodes],
        True,
    )

    attachments_per_render_pass = [
        [link.from_node for link in node.inputs["Attachments"].links]
        for node in render_pass_nodes
    ]

    assert functools.reduce(
        operator.and_,
        [
            attachment.bl_idname == "AttachmentNode"
            for attachments in attachments_per_render_pass
            for attachment in attachments
        ],
        True,
    )

    subpasses_per_render_pass = [
        [link.from_node for link in node.inputs["Subpasses"].links]
        for node in render_pass_nodes
    ]

    assert functools.reduce(
        operator.and_,
        [
            subpass.bl_idname == "SubpassNode"
            for subpasses in subpasses_per_render_pass
            for subpass in subpasses
        ],
        True,
    )

    subpass_dependencies_per_render_pass = [
        [link.from_node for link in node.inputs["Dependencies"].links]
        for node in render_pass_nodes
    ]

    assert functools.reduce(
        operator.and_,
        [
            subpass_dependency.bl_idname == "SubpassDependencyNode"
            for subpass_dependencies in subpass_dependencies_per_render_pass
            for subpass_dependency in subpass_dependencies
        ],
        True,
    )

    attachments_per_render_pass_jsons = [
        [attachment_to_json(attachment) for attachment in attachments]
        for attachments in attachments_per_render_pass
    ]

    subpasses_per_render_pass_jsons = [
        [
            subpass_to_json(subpass, attachments_per_render_pass[render_pass_index])
            for subpass in subpasses
        ]
        for render_pass_index, subpasses in enumerate(subpasses_per_render_pass)
    ]

    dependencies_per_render_pass_jsons = [
        [
            subpass_dependency_to_json(
                subpass_dependency, subpasses_per_render_pass[render_pass_index]
            )
            for subpass_dependency in subpass_dependencies
        ]
        for render_pass_index, subpass_dependencies in enumerate(
            subpass_dependencies_per_render_pass
        )
    ]

    json = [
        {
            "name": name,
            "attachments": attachment_jsons,
            "subpasses": subpass_jsons,
            "dependencies": dependency_jsons,
        }
        for (name, attachment_jsons, subpass_jsons, dependency_jsons) in zip(
            render_pass_names,
            attachments_per_render_pass_jsons,
            subpasses_per_render_pass_jsons,
            dependencies_per_render_pass_jsons,
        )
    ]

    return (render_pass_nodes, subpasses_per_render_pass, json)
