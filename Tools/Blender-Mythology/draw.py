import bpy

from .common import Rect2DNodeSocket
from .render_node_tree import RenderTreeNode
from .resources import ImageNodeSocket, ImageSubresourceRangeNodeSocket
from .vulkan_enums import access_flag_values, dependency_flag_values, image_layout_values, pipeline_stage_flag_values

class ClearColorValueNodeSocket(bpy.types.NodeSocket):
    
    bl_label = "Clear Color Value node socket"

    def draw(self, context, layout, node, text):
        layout.label(text=text)

    def draw_color(self, context, node):
        return (1.0, 0.0, 0.0, 1.0)

class ExecutionNodeSocket(bpy.types.NodeSocket):
    
    bl_label = "Execution node socket"

    def draw(self, context, layout, node, text):
        layout.label(text=text)

    def draw_color(self, context, node):
        return (1.0, 1.0, 1.0, 1.0)

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

class EndFrameNode(bpy.types.Node, RenderTreeNode):

    bl_label = "End Frame node"

    def init(self, context):
        self.inputs.new("ExecutionNodeSocket", "Execution")

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
        layout.prop(self, "dependency_flags_property")

import nodeitems_utils

class DrawNodeCategory(nodeitems_utils.NodeCategory):
    
    @classmethod
    def poll(cls, context):
        return context.space_data.tree_type == "RenderNodeTree"


draw_node_categories = [
    DrawNodeCategory("COMMANDS", "Draw", items=[
        nodeitems_utils.NodeItem("BeginFrameNode"),
        nodeitems_utils.NodeItem("ClearColorValueNode"),
        nodeitems_utils.NodeItem("ClearColorImageNode"),
        nodeitems_utils.NodeItem("EndFrameNode"),
        nodeitems_utils.NodeItem("ImageMemoryBarrierNode"),
        nodeitems_utils.NodeItem("PipelineBarrierNode"),
    ]),
]
