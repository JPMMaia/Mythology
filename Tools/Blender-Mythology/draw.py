import bpy

from .array_inputs import recreate_dynamic_inputs, update_dynamic_inputs
from .common import find_index, Rect2DNodeSocket
from .pipeline_state import GraphicsPipelineStateNode, PipelineNodeSocket
from .render_node_tree import RenderTreeNode
from .render_pass import (
    RenderPassNode,
    RenderPassNodeSocket,
    SubpassNode,
    SubpassNodeSocket,
)
from .ray_tracing import ShaderBindingTableNode
from .resources import ImageNodeSocket, ImageSubresourceRangeNodeSocket
from .vulkan_enums import (
    access_flag_values,
    dependency_flag_values,
    image_layout_values,
    image_layout_values_to_int,
    pipeline_bind_point_values,
    get_pipeline_bind_point_value,
    pipeline_stage_flag_values,
)


class ClearColorValueNodeSocket(bpy.types.NodeSocket):

    bl_label = "Clear Color Value node socket"

    def draw(self, context, layout, node, text):
        layout.label(text=text)

    def draw_color(self, context, node):
        return (1.0, 0.0, 0.0, 1.0)


class ClearDepthStencilValueNodeSocket(bpy.types.NodeSocket):

    bl_label = "Clear Depth Stencil Value node socket"

    def draw(self, context, layout, node, text):
        layout.label(text=text)

    def draw_color(self, context, node):
        return (0.0, 0.0, 1.0, 1.0)


class ClearSubpassNodeSocket(bpy.types.NodeSocket):

    bl_label = "Clear Subpass node socket"

    def draw(self, context, layout, node, text):
        layout.label(text=text)

    def draw_color(self, context, node):
        return (0.0, 1.0, 0.0, 1.0)


class ClearValueNodeSocket(bpy.types.NodeSocket):

    bl_label = "Clear Value node socket"

    def draw(self, context, layout, node, text):
        layout.label(text=text)

    def draw_color(self, context, node):
        return (1.0, 1.0, 1.0, 1.0)

class DescriptorSetNodeSocket(bpy.types.NodeSocket):

    bl_label = "Descriptor Set node socket"

    def draw(self, context, layout, node, text):
        layout.label(text=text)

    def draw_color(self, context, node):
        return (0.5, 0.9, 0.7, 1.0)

class DescriptorSetArrayNodeSocket(bpy.types.NodeSocket):

    bl_label = "Descriptor Set Array node socket"

    def draw(self, context, layout, node, text):
        layout.label(text=text)

    def draw_color(self, context, node):
        return (0.9, 0.7, 0.5, 1.0)

class DynamicOffsetNodeSocket(bpy.types.NodeSocket):

    bl_label = "Dynamic Offset node socket"

    def draw(self, context, layout, node, text):
        layout.label(text=text)

    def draw_color(self, context, node):
        return (0.1, 0.7, 0.4, 1.0)


class DynamicOffsetArrayNodeSocket(bpy.types.NodeSocket):

    bl_label = "Dynamic Offset Array node socket"

    def draw(self, context, layout, node, text):
        layout.label(text=text)

    def draw_color(self, context, node):
        return (0.7, 0.2, 0.2, 1.0)


class ExecutionNodeSocket(bpy.types.NodeSocket):

    bl_label = "Execution node socket"

    def draw(self, context, layout, node, text):
        layout.label(text=text)

    def draw_color(self, context, node):
        return (1.0, 1.0, 1.0, 1.0)


class FramebufferNodeSocket(bpy.types.NodeSocket):

    bl_label = "Framebuffer node socket"

    def draw(self, context, layout, node, text):
        layout.label(text=text)

    def draw_color(self, context, node):
        return (0.5, 0.5, 0.5, 1.0)


class ImageMemoryBarrierNodeSocket(bpy.types.NodeSocket):

    bl_label = "Image Memory Barrier node socket"

    def draw(self, context, layout, node, text):
        layout.label(text=text)

    def draw_color(self, context, node):
        return (0.0, 1.0, 0.0, 1.0)


class BeginFrameNode(bpy.types.Node, RenderTreeNode):

    bl_label = "Begin Frame node"

    def init(self, context):
        self.outputs.new("ExecutionNodeSocket", "Execution")
        self.outputs.new("ImageNodeSocket", "Output Image")
        self.outputs.new(
            "ImageSubresourceRangeNodeSocket", "Output Image Subresource Range"
        )
        self.outputs.new("Rect2DNodeSocket", "Output Image Area")
        self.outputs.new("FramebufferNodeSocket", "Output Framebuffer")


class BeginRenderPassNode(bpy.types.Node, RenderTreeNode):

    bl_label = "Begin Render Pass node"

    def init(self, context):
        self.inputs.new("ExecutionNodeSocket", "Execution")
        self.inputs.new("RenderPassNodeSocket", "Render Pass")
        self.inputs.new("FramebufferNodeSocket", "Framebuffer")
        self.inputs.new("Rect2DNodeSocket", "Render Area")
        self.inputs.new("ClearSubpassNodeSocket", "Clear Subpasses")
        self.inputs["Clear Subpasses"].link_limit = 0

        self.outputs.new("ExecutionNodeSocket", "Execution")


class BindDescriptorSetNode(bpy.types.Node, RenderTreeNode):

    bl_label = "Bind Descriptor Set node"

    pipeline_bind_point_property: bpy.props.EnumProperty(
        name="Pipeline Bind Point", items=pipeline_bind_point_values
    )
    first_set_property: bpy.props.IntProperty(name="First Set", min=0)

    def init(self, context):
        self.inputs.new("ExecutionNodeSocket", "Execution")
        self.inputs.new("PipelineLayoutNodeSocket", "Pipeline Layout")
        self.inputs.new("DescriptorSetArrayNodeSocket", "Descriptor Set Array")
        self.inputs.new("DynamicOffsetArrayNodeSocket", "Dynamic Offset Array")
        self.outputs.new("ExecutionNodeSocket", "Execution")

    def draw_buttons(self, context, layout):
        layout.prop(self, "pipeline_bind_point_property")
        layout.prop(self, "first_set_property")


class BindPipelineNode(bpy.types.Node, RenderTreeNode):

    bl_label = "Bind Pipeline node"

    pipeline_bind_point_property: bpy.props.EnumProperty(
        name="Pipeline Bind Point", items=pipeline_bind_point_values
    )

    def init(self, context):
        self.inputs.new("ExecutionNodeSocket", "Execution")
        self.inputs.new("PipelineNodeSocket", "Pipeline")
        self.outputs.new("ExecutionNodeSocket", "Execution")

    def draw_buttons(self, context, layout):
        layout.prop(self, "pipeline_bind_point_property")


class ClearColorValueNode(bpy.types.Node, RenderTreeNode):

    bl_label = "Clear Color Value node"

    type_property: bpy.props.EnumProperty(
        name="Type",
        items=[
            ("FLOAT", "Float", "", 0),
            ("INT", "Int", "", 1),
            ("UINT", "Uint", "", 2),
        ],
    )
    float_values_property: bpy.props.FloatVectorProperty(name="Float_32", size=4)
    int_values_property: bpy.props.IntVectorProperty(name="Int_32", size=4)
    uint_values_property: bpy.props.IntVectorProperty(name="Uint_32", min=0, size=4)

    def init(self, context):
        self.outputs.new("ClearColorValueNodeSocket", "Clear Color Value")

    def draw_buttons(self, context, layout):

        layout.prop(self, "type_property")

        if self.type_property == "FLOAT":
            layout.prop(self, "float_values_property")

        elif self.type_property == "INT":
            layout.prop(self, "int_values_property")

        elif self.type_property == "UINT":
            layout.prop(self, "uint_values_property")


class ClearColorImageNode(bpy.types.Node, RenderTreeNode):

    bl_label = "Clear Color Image node"

    def init(self, context):
        self.inputs.new("ExecutionNodeSocket", "Execution")
        self.inputs.new("ImageNodeSocket", "Image")
        self.inputs.new("ImageSubresourceRangeNodeSocket", "Image Subresource Ranges")
        self.inputs.new("ClearColorValueNodeSocket", "Clear Color Value")
        self.inputs["Image Subresource Ranges"].link_limit = 0

        self.outputs.new("ExecutionNodeSocket", "Execution")


class ClearDepthStencilValueNode(bpy.types.Node, RenderTreeNode):

    bl_label = "Clear Depth Stencil Value node"

    depth_property: bpy.props.FloatVectorProperty(name="Depth")
    stencil_property: bpy.props.IntVectorProperty(name="Stencil", min=0)

    def init(self, context):
        self.outputs.new(
            "ClearDepthStencilValueNodeSocket", "Clear Depth Stencil Value"
        )

    def draw_buttons(self, context, layout):

        layout.prop(self, "depth_property")
        layout.prop(self, "stencil_property")


class ClearSubpassNode(bpy.types.Node, RenderTreeNode):

    bl_label = "Clear Subpass node"

    def init(self, context):
        self.inputs.new("ClearValueNodeSocket", "Clear Value")
        self.inputs.new("SubpassNodeSocket", "Subpass")

        self.outputs.new("ClearSubpassNodeSocket", "Clear Subpass")


class ClearValueNode(bpy.types.Node, RenderTreeNode):

    bl_label = "Clear Value node"

    def init(self, context):
        self.inputs.new("ClearColorValueNodeSocket", "Color")
        self.inputs.new("ClearDepthStencilValueNodeSocket", "Depth Stencil")

        self.outputs.new("ClearValueNodeSocket", "Clear Value")


class DynamicOffsetNode(bpy.types.Node, RenderTreeNode):

    bl_label = "Dynamic Offset"
    value_property: bpy.props.IntProperty(name="Value", min=0)

    def init(self, context):
        self.outputs.new("DynamicOffsetNodeSocket", "Dynamic Offset")

    def draw_buttons(self, context, layout):
        layout.prop(self, "value_property")


class DynamicOffsetArrayNode(bpy.types.Node, RenderTreeNode):

    bl_label = "Dynamic Offsets Array"
    recreating = False

    def init(self, context):
        self.recreating = True
        recreate_dynamic_inputs(self.id_data, self.inputs, "DynamicOffsetNodeSocket")
        self.recreating = False

        self.outputs.new("DynamicOffsetArrayNodeSocket", "Dynamic Offsets Array")

    def update(self):
        if not self.recreating:
            self.recreating = True
            update_dynamic_inputs(self.id_data, self.inputs, "DynamicOffsetNodeSocket")
            self.recreating = False


class DrawNode(bpy.types.Node, RenderTreeNode):

    bl_label = "Draw node"

    vertex_count_property: bpy.props.IntProperty(name="Vertex Count", default=1, min=1)
    instance_count_property: bpy.props.IntProperty(
        name="Instance Count", default=1, min=1
    )
    first_vertex_property: bpy.props.IntProperty(name="First Vertex", min=0)
    first_instance_property: bpy.props.IntProperty(name="First Instance", min=0)

    def init(self, context):
        self.inputs.new("ExecutionNodeSocket", "Execution")
        self.outputs.new("ExecutionNodeSocket", "Execution")

    def draw_buttons(self, context, layout):

        layout.prop(self, "vertex_count_property")
        layout.prop(self, "instance_count_property")
        layout.prop(self, "first_vertex_property")
        layout.prop(self, "first_instance_property")


class EndFrameNode(bpy.types.Node, RenderTreeNode):

    bl_label = "End Frame node"

    def init(self, context):
        self.inputs.new("ExecutionNodeSocket", "Execution")


class EndRenderPassNode(bpy.types.Node, RenderTreeNode):

    bl_label = "End Render Pass node"

    def init(self, context):
        self.inputs.new("ExecutionNodeSocket", "Execution")
        self.outputs.new("ExecutionNodeSocket", "Execution")


class ImageMemoryBarrierNode(bpy.types.Node, RenderTreeNode):

    bl_label = "Image Memory Barrier node"

    source_access_mask_property: bpy.props.EnumProperty(
        name="Source access mask", items=access_flag_values, options={"ENUM_FLAG"}
    )
    destination_access_mask_property: bpy.props.EnumProperty(
        name="Destination access mask", items=access_flag_values, options={"ENUM_FLAG"}
    )
    old_layout_property: bpy.props.EnumProperty(
        name="Old layout", items=image_layout_values
    )
    new_layout_property: bpy.props.EnumProperty(
        name="New layout", items=image_layout_values
    )

    def init(self, context):
        self.inputs.new("ImageNodeSocket", "Image")
        self.inputs.new("ImageSubresourceRangeNodeSocket", "Image Subresource Range")

        self.outputs.new("ImageMemoryBarrierNodeSocket", "Image Memory Barrier")

    def draw_buttons(self, context, layout):

        layout.label(text="Source Access Mask")
        layout.prop(self, "source_access_mask_property")
        layout.label(text="Destination Access Mask")
        layout.prop(self, "destination_access_mask_property")
        layout.prop(self, "old_layout_property")
        layout.prop(self, "new_layout_property")


class PipelineBarrierNode(bpy.types.Node, RenderTreeNode):

    bl_label = "Pipeline Barrier node"

    source_stage_mask_property: bpy.props.EnumProperty(
        name="Source stage mask",
        items=pipeline_stage_flag_values,
        options={"ENUM_FLAG"},
    )
    destination_stage_mask_property: bpy.props.EnumProperty(
        name="Destination stage mask",
        items=pipeline_stage_flag_values,
        options={"ENUM_FLAG"},
    )
    dependency_flags_property: bpy.props.EnumProperty(
        name="Dependency flags", items=dependency_flag_values, options={"ENUM_FLAG"}
    )

    def init(self, context):
        self.inputs.new("ExecutionNodeSocket", "Execution")
        # TODO memory barriers
        # TODO buffer barriers
        self.inputs.new("ImageMemoryBarrierNodeSocket", "Image Barriers")
        self.inputs["Image Barriers"].link_limit = 0

        self.outputs.new("ExecutionNodeSocket", "Execution")

    def draw_buttons(self, context, layout):

        layout.label(text="Source Stage Mask")
        layout.prop(self, "source_stage_mask_property")
        layout.label(text="Destination Stage Mask")
        layout.prop(self, "destination_stage_mask_property")
        layout.label(text="Dependency Flags")
        layout.prop(self, "dependency_flags_property")


class SetScreenViewportAndScissorsNode(bpy.types.Node, RenderTreeNode):

    bl_label = "Set Screen Viewport And Scissors node"

    def init(self, context):
        self.inputs.new("ExecutionNodeSocket", "Execution")
        self.outputs.new("ExecutionNodeSocket", "Execution")


class TraceRaysNode(bpy.types.Node, RenderTreeNode):

    bl_label = "Trace Rays"

    width_property: bpy.props.IntProperty(name="Width", default=1, min=1)
    height_property: bpy.props.IntProperty(name="Heighth", default=1, min=1)
    depth_property: bpy.props.IntProperty(name="Depth", default=1, min=1)

    def init(self, context):
        self.inputs.new("ExecutionNodeSocket", "Execution")
        self.inputs.new("ShaderBindingTableNodeSocket", "Raygen Shader Binding Table")
        self.inputs.new("ShaderBindingTableNodeSocket", "Miss Shader Binding Table")
        self.inputs.new("ShaderBindingTableNodeSocket", "Hit Shader Binding Table")
        self.inputs.new("ShaderBindingTableNodeSocket", "Callable Shader Binding Table")

        self.outputs.new("ExecutionNodeSocket", "Execution")

    def draw_buttons(self, context, layout):

        layout.prop(self, "width_property")
        layout.prop(self, "height_property")
        layout.prop(self, "depth_property")


import nodeitems_utils


class DrawNodeCategory(nodeitems_utils.NodeCategory):
    @classmethod
    def poll(cls, context):
        return context.space_data.tree_type == "RenderNodeTree"


draw_node_categories = [
    DrawNodeCategory(
        "COMMANDS",
        "Draw",
        items=[
            nodeitems_utils.NodeItem("BeginFrameNode"),
            nodeitems_utils.NodeItem("BeginRenderPassNode"),
            nodeitems_utils.NodeItem("BindDescriptorSetNode"),
            nodeitems_utils.NodeItem("BindPipelineNode"),
            nodeitems_utils.NodeItem("DynamicOffsetNode"),
            nodeitems_utils.NodeItem("DynamicOffsetArrayNode"),
            nodeitems_utils.NodeItem("ClearColorValueNode"),
            nodeitems_utils.NodeItem("ClearColorImageNode"),
            nodeitems_utils.NodeItem("ClearDepthStencilValueNode"),
            nodeitems_utils.NodeItem("ClearSubpassNode"),
            nodeitems_utils.NodeItem("ClearValueNode"),
            nodeitems_utils.NodeItem("DrawNode"),
            nodeitems_utils.NodeItem("EndFrameNode"),
            nodeitems_utils.NodeItem("EndRenderPassNode"),
            nodeitems_utils.NodeItem("ImageMemoryBarrierNode"),
            nodeitems_utils.NodeItem("PipelineBarrierNode"),
            nodeitems_utils.NodeItem("SetScreenViewportAndScissorsNode"),
            nodeitems_utils.NodeItem("TraceRaysNode"),
        ],
    ),
]

import typing
from .common import ignore_reroutes

JSONType = typing.Union[
    str, int, float, bool, None, typing.Dict[str, typing.Any], typing.List[typing.Any]
]


def begin_render_pass_node_to_json(
    node: BeginRenderPassNode,
    render_passes: typing.Tuple[
        typing.List[RenderPassNode], typing.List[typing.List[SubpassNode]], JSONType
    ],
) -> JSONType:

    assert (
        ignore_reroutes(node.inputs["Framebuffer"].links[0].from_node).bl_idname
        == "BeginFrameNode"
    )
    assert (
        ignore_reroutes(node.inputs["Render Area"].links[0].from_node).bl_idname
        == "BeginFrameNode"
    )

    return {
        "type": "Begin_render_pass",
        "subtype": "Dependent",
        "render_pass": render_passes[0].index(
            ignore_reroutes(node.inputs["Render Pass"].links[0].from_node)
        ),
    }


def bind_pipeline_node_to_json(
    node: BindPipelineNode,
    pipeline_states: typing.List[bpy.types.Node],
) -> JSONType:

    return {
        "type": "Bind_pipeline",
        "pipeline_bind_point": get_pipeline_bind_point_value(
            node.get("pipeline_bind_point_property", 0)
        ),
        "pipeline": pipeline_states.index(
            ignore_reroutes(node.inputs["Pipeline"].links[0].from_node)
        ),
    }


def clear_color_value_node_to_json(node: ClearColorValueNode) -> JSONType:

    values_property = (
        node.float_values_property
        if node.type_property == "FLOAT"
        else (
            node.int_values_property
            if node.type_property == "INT"
            else node.uint_values_property
        )
    )

    return {"type": node.type_property, "values": [value for value in values_property]}


def clear_color_image_node_to_json(node: ClearColorImageNode) -> JSONType:

    assert (
        ignore_reroutes(node.inputs["Image"].links[0].from_node).bl_idname
        == "BeginFrameNode"
    )
    assert (
        ignore_reroutes(
            node.inputs["Image Subresource Ranges"].links[0].from_node
        ).bl_idname
        == "BeginFrameNode"
    )

    return {
        "type": "Clear_color_image",
        "subtype": "Dependent",
        "clear_color_value": clear_color_value_node_to_json(
            ignore_reroutes(node.inputs["Clear Color Value"].links[0].from_node)
        ),
    }


def draw_node_to_json(node: DrawNode) -> JSONType:

    return {
        "type": "Draw",
        "vertex_count": node.get("vertex_count_property", 1),
        "instance_count": node.get("instance_count_property", 1),
        "first_vertex": node.get("first_vertex_property", 0),
        "first_instance": node.get("first_instance_property", 0),
    }


def end_render_pass_node_to_json(node: EndRenderPassNode) -> JSONType:

    return {"type": "End_render_pass"}


def image_layout_to_int(value: str) -> int:

    return image_layout_values_to_int[value]


def image_memory_barrier_node_to_json(node: ImageMemoryBarrierNode) -> JSONType:

    assert (
        ignore_reroutes(node.inputs["Image"].links[0].from_node).bl_idname
        == "BeginFrameNode"
    )
    assert (
        ignore_reroutes(
            node.inputs["Image Subresource Range"].links[0].from_node
        ).bl_idname
        == "BeginFrameNode"
    )

    return {
        "type": "Dependent",
        "source_access_mask": node.get("source_access_mask_property", 0),
        "destination_access_mask": node.get("destination_access_mask_property", 0),
        "old_layout": image_layout_to_int(node.old_layout_property),
        "new_layout": image_layout_to_int(node.new_layout_property),
    }


def pipeline_barrier_node_to_json(node: PipelineBarrierNode) -> JSONType:

    return {
        "type": "Pipeline_barrier",
        "source_stage_mask": node.get("source_stage_mask_property", 0),
        "destination_stage_mask": node.get("destination_stage_mask_property", 0),
        "dependency_flags": node.get("dependency_flags_property", 0),
        "memory_barriers": [],  # TODO
        "buffer_barriers": [],  # TODO
        "image_barriers": [
            image_memory_barrier_node_to_json(link.from_node)
            for link in node.inputs["Image Barriers"].links
        ],
    }


def set_screen_viewport_and_scissors_node_to_json(
    node: SetScreenViewportAndScissorsNode,
) -> JSONType:

    return {"type": "Set_screen_viewport_and_scissors"}


def trace_rays_node_to_json(
    node: TraceRaysNode, shader_binding_table_nodes: typing.List[ShaderBindingTableNode]
) -> JSONType:

    json = {
        "type": "Trace_rays",
        "width": node.get("width_property", 1),
        "height": node.get("height_property", 1),
        "depth": node.get("depth_property", 1),
    }

    if len(node.inputs["Raygen Shader Binding Table"].links) == 1:
        json["raygen_shader_binding_table_index"] = find_index(
            shader_binding_table_nodes,
            node.inputs["Raygen Shader Binding Table"].links[0].from_node,
        )

    if len(node.inputs["Miss Shader Binding Table"].links) == 1:
        json["miss_shader_binding_table_index"] = find_index(
            shader_binding_table_nodes,
            node.inputs["Miss Shader Binding Table"].links[0].from_node,
        )

    if len(node.inputs["Hit Shader Binding Table"].links) == 1:
        json["hit_shader_binding_table_index"] = find_index(
            shader_binding_table_nodes,
            node.inputs["Hit Shader Binding Table"].links[0].from_node,
        )

    if len(node.inputs["Callable Shader Binding Table"].links) == 1:
        json["callable_shader_binding_table_index"] = find_index(
            shader_binding_table_nodes,
            node.inputs["Callable Shader Binding Table"].links[0].from_node,
        )

    return json


def frame_command_node_to_json(
    node: bpy.types.Node,
    pipeline_states: typing.List[bpy.types.Node],
    render_passes: typing.Tuple[
        typing.List[RenderPassNode], typing.List[typing.List[SubpassNode]], JSONType
    ],
    shader_binding_tables: typing.List[ShaderBindingTableNode],
) -> JSONType:

    if node.bl_idname == "BeginRenderPassNode":
        return begin_render_pass_node_to_json(node, render_passes)
    elif node.bl_idname == "BindPipelineNode":
        return bind_pipeline_node_to_json(node, pipeline_states)
    elif node.bl_idname == "ClearColorImageNode":
        return clear_color_image_node_to_json(node)
    elif node.bl_idname == "DrawNode":
        return draw_node_to_json(node)
    elif node.bl_idname == "EndRenderPassNode":
        return end_render_pass_node_to_json(node)
    elif node.bl_idname == "PipelineBarrierNode":
        return pipeline_barrier_node_to_json(node)
    elif node.bl_idname == "SetScreenViewportAndScissorsNode":
        return set_screen_viewport_and_scissors_node_to_json(node)
    elif node.bl_idname == "TraceRaysNode":
        return trace_rays_node_to_json(node, shader_binding_tables)

    assert False


def get_frame_command_nodes(begin_frame_node: BeginFrameNode) -> [bpy.types.Node]:

    frame_command_nodes = []

    current_node = begin_frame_node
    while (
        current_node.outputs["Execution"].links[0].to_node.bl_idname != "EndFrameNode"
    ):

        next_node = current_node.outputs["Execution"].links[0].to_node
        frame_command_nodes.append(next_node)

        current_node = next_node

    return frame_command_nodes


def frame_commands_to_json(
    nodes: typing.List[bpy.types.Node],
    pipeline_states: typing.List[bpy.types.Node],
    render_passes: typing.Tuple[
        typing.List[RenderPassNode], typing.List[typing.List[SubpassNode]], JSONType
    ],
    shader_binding_tables: typing.List[ShaderBindingTableNode],
) -> JSONType:

    begin_frame_nodes = [node for node in nodes if node.bl_idname == "BeginFrameNode"]

    frame_command_nodes_per_begin_frame_node = [
        get_frame_command_nodes(node) for node in begin_frame_nodes
    ]

    return [
        [
            frame_command_node_to_json(
                command_node, pipeline_states, render_passes, shader_binding_tables
            )
            for command_node in frame_command_nodes_per_begin_frame_node[
                begin_frame_index
            ]
        ]
        for begin_frame_index, begin_frame_node in enumerate(begin_frame_nodes)
    ]
