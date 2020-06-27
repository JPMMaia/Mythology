import bpy

from .common import Rect2DNodeSocket
from .pipeline_state import PipelineNodeSocket
from .render_node_tree import RenderTreeNode
from .render_pass import RenderPassNodeSocket, SubpassNodeSocket
from .resources import ImageNodeSocket, ImageSubresourceRangeNodeSocket
from .vulkan_enums import access_flag_values, dependency_flag_values, image_layout_values, image_layout_values_to_int, pipeline_bind_point_values, pipeline_stage_flag_values


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
        self.outputs.new("ImageSubresourceRangeNodeSocket", "Output Image Subresource Range")
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

class BindPipelineNode(bpy.types.Node, RenderTreeNode):

    bl_label = "Bind Pipeline node"

    pipeline_bind_point_property: bpy.props.EnumProperty(name="Pipeline Bind Point", items=pipeline_bind_point_values)

    def init(self, context):
        self.inputs.new("ExecutionNodeSocket", "Execution")
        self.inputs.new("PipelineNodeSocket", "Pipeline")
        self.outputs.new("ExecutionNodeSocket", "Execution")

    def draw_buttons(self, context, layout):
        layout.prop(self, "pipeline_bind_point_property")

class ClearColorValueNode(bpy.types.Node, RenderTreeNode):

    bl_label = "Clear Color Value node"

    type_property: bpy.props.EnumProperty(name="Type", items=[("FLOAT", "Float", "", 0), ("INT", "Int", "", 1), ("UINT", "Uint", "", 2)])
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
        self.outputs.new("ClearDepthStencilValueNodeSocket", "Clear Depth Stencil Value")

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
    


class DrawNode(bpy.types.Node, RenderTreeNode):

    bl_label = "Draw node"

    vertex_count_property: bpy.props.IntProperty(name="Vertex Count", default=1, min=1)
    instance_count_property: bpy.props.IntProperty(name="Instance Count", default=1, min=1)
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

    source_access_mask_property: bpy.props.EnumProperty(name="Source access mask", items=access_flag_values, options={"ENUM_FLAG"})
    destination_access_mask_property: bpy.props.EnumProperty(name="Destination access mask", items=access_flag_values, options={"ENUM_FLAG"})
    old_layout_property: bpy.props.EnumProperty(name="Old layout", items=image_layout_values)
    new_layout_property: bpy.props.EnumProperty(name="New layout", items=image_layout_values)

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

    source_stage_mask_property: bpy.props.EnumProperty(name="Source stage mask", items=pipeline_stage_flag_values, options={"ENUM_FLAG"})
    destination_stage_mask_property: bpy.props.EnumProperty(name="Destination stage mask", items=pipeline_stage_flag_values, options={"ENUM_FLAG"})
    dependency_flags_property: bpy.props.EnumProperty(name="Dependency flags", items=dependency_flag_values, options={"ENUM_FLAG"})

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

import nodeitems_utils

class DrawNodeCategory(nodeitems_utils.NodeCategory):
    
    @classmethod
    def poll(cls, context):
        return context.space_data.tree_type == "RenderNodeTree"


draw_node_categories = [
    DrawNodeCategory("COMMANDS", "Draw", items=[
        nodeitems_utils.NodeItem("BeginFrameNode"),
        nodeitems_utils.NodeItem("BeginRenderPassNode"),
        nodeitems_utils.NodeItem("BindPipelineNode"),
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
    ]),
]

import typing

JSONType = typing.Union[str, int, float, bool, None, typing.Dict[str, typing.Any], typing.List[typing.Any]]

def clear_color_value_node_to_json(
    node: ClearColorValueNode
) -> JSONType:

    values_property = node.float_values_property if node.type_property == "FLOAT" else (
                      node.int_values_property if node.type_property == "INT" else
                      node.uint_values_property)

    return {
        "type": node.type_property,
        "values": [value for value in values_property]
    }

def clear_color_image_node_to_json(
    node: ClearColorImageNode
) -> JSONType:

    assert node.inputs["Image"].links[0].from_node.bl_idname == "BeginFrameNode"
    assert node.inputs["Image Subresource Ranges"].links[0].from_node.bl_idname == "BeginFrameNode"

    return {
        "type": "Clear_color_image",
        "subtype": "Dependent",
        "clear_color_value": clear_color_value_node_to_json(node.inputs["Clear Color Value"].links[0].from_node),
    }

def image_layout_to_int(
    value: str
) -> int:

    return image_layout_values_to_int[value]

def image_memory_barrier_node_to_json(
    node: ImageMemoryBarrierNode
) -> JSONType:

    assert node.inputs["Image"].links[0].from_node.bl_idname == "BeginFrameNode"
    assert node.inputs["Image Subresource Range"].links[0].from_node.bl_idname == "BeginFrameNode"

    return {
        "type": "Dependent",
        "source_access_mask": node.get("source_access_mask_property", 0),
        "destination_access_mask": node.get("destination_access_mask_property", 0),
        "old_layout": image_layout_to_int(node.old_layout_property),
        "new_layout": image_layout_to_int(node.new_layout_property),
    }

def pipeline_barrier_node_to_json(
    node: PipelineBarrierNode
) -> JSONType:

    return {
        "type": "Pipeline_barrier",
        "source_stage_mask": node.get("source_stage_mask_property", 0),
        "destination_stage_mask": node.get("destination_stage_mask_property", 0),
        "dependency_flags": node.get("dependency_flags_property", 0),
        "memory_barriers": [], # TODO
        "buffer_barriers": [], # TODO
        "image_barriers": [image_memory_barrier_node_to_json(link.from_node)
                           for link in node.inputs["Image Barriers"].links],
    }

def frame_command_node_to_json(
    node: bpy.types.Node
) -> JSONType:

    if node.bl_idname == "ClearColorImageNode":
        return clear_color_image_node_to_json(node)
    elif node.bl_idname == "PipelineBarrierNode":
        return pipeline_barrier_node_to_json(node)

    assert False

def get_frame_command_nodes(
    begin_frame_node: BeginFrameNode
) -> [bpy.types.Node]:

    frame_command_nodes = []

    current_node = begin_frame_node
    while current_node.outputs["Execution"].links[0].to_node.bl_idname != "EndFrameNode":

        next_node = current_node.outputs["Execution"].links[0].to_node
        frame_command_nodes.append(next_node)
        
        current_node = next_node

    return frame_command_nodes

def frame_commands_to_json(
    nodes: typing.List[bpy.types.Node]
) -> JSONType:

    begin_frame_nodes = [node
                         for node in nodes
                         if node.bl_idname == "BeginFrameNode"]

    frame_command_nodes_per_begin_frame_node = [get_frame_command_nodes(node)
                                                for node in begin_frame_nodes]
    
    return [
        [
            frame_command_node_to_json(command_node)
            for command_node in frame_command_nodes_per_begin_frame_node[begin_frame_index]
        ]
        for begin_frame_index, begin_frame_node in enumerate(begin_frame_nodes)
    ]
