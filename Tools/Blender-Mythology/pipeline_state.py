import bpy

from .render_node_tree import RenderTreeNode
from .vulkan_enums import *

class PipelineShaderStageNodeSocket(bpy.types.NodeSocket):
    
    bl_label = "Pipeline Shader Node Socket"

    def draw(self, context, layout, node, text):
        layout.label(text=text)

    def draw_color(self, context, node):
        return (0.0, 1.0, 0.0, 1.0)

class ShaderModuleNodeSocket(bpy.types.NodeSocket):
    
    bl_label = "Shader Module Node Socket"

    def draw(self, context, layout, node, text):
        layout.label(text=text)

    def draw_color(self, context, node):
        return (1.0, 0.0, 0.0, 1.0)

class VertexInputAttributeNodeSocket(bpy.types.NodeSocket):

    bl_label = "Vertex Input Attribute node socket"

    def draw(self, context, layout, node, text):
        layout.label(text=text)

    def draw_color(self, context, node):
        return (0.0, 1.0, 1.0, 1.0)

class VertexInputBindingNodeSocket(bpy.types.NodeSocket):

    bl_label = "Vertex Input Binding node socket"

    def draw(self, context, layout, node, text):
        layout.label(text=text)

    def draw_color(self, context, node):
        return (1.0, 0.0, 1.0, 1.0)

class VertexInputStateNodeSocket(bpy.types.NodeSocket):

    bl_label = "Vertex Input State node socket"

    def draw(self, context, layout, node, text):
        layout.label(text=text)

    def draw_color(self, context, node):
        return (0.0, 0.0, 1.0, 1.0)


class PipelineShaderStageNode(bpy.types.Node, RenderTreeNode):

    bl_label = 'Pipeline Shader Stage node'

    def init(self, context):
        self.inputs.new("ShaderModuleNodeSocket", "Shader")
        self.outputs.new("PipelineShaderStageNodeSocket", "Stage")


shader_type_values = [
    ("VS", "Vertex Shader", "", 0),
    ("GS", "Geometry Shader", "", 1),
    ("HS", "Hull Shader", "", 2),
    ("DS", "Domain Shader", "", 3),
    ("PS", "Pixel Shader", "", 4),
    ("CS", "Compute Shader", "", 5),
    ("MS", "Mesh Shader", "", 6),
    ("AS", "Amplification Shader", "", 7),
]

class ShaderModuleNode(bpy.types.Node, RenderTreeNode):

    bl_label = 'Shader Module node'

    shader_file: bpy.props.StringProperty(name="File", subtype="FILE_PATH")
    language: bpy.props.EnumProperty(items=[("HLSL", "HLSL", "", 0)])
    shader_type: bpy.props.EnumProperty(name="Type", items=shader_type_values)
    shader_model: bpy.props.StringProperty(name="Model", default="6_5")
    entry_point: bpy.props.StringProperty(name="Entry point", default="main")
    additional_compile_flags: bpy.props.StringProperty(name="Compile flags", description="Additional compile flags")

    def init(self, context):
        self.outputs.new("ShaderModuleNodeSocket", "Shader")

    def draw_buttons(self, context, layout):
        layout.prop(self, "shader_file")
        layout.prop(self, "language")
        layout.prop(self, "shader_type")
        layout.prop(self, "shader_model")
        layout.prop(self, "entry_point")
        layout.prop(self, "additional_compile_flags")


class VertexInputAttributeNode(bpy.types.Node, RenderTreeNode):

    bl_label = 'Vertex Input Attribute node'

    location_property: bpy.props.IntProperty(name="Location", min=0)
    binding_property: bpy.props.IntProperty(name="Binding", min=0)
    format_property: bpy.props.EnumProperty(name="Format", items=format_values)
    offset_property: bpy.props.IntProperty(name="Offset", min=0)

    def init(self, context):
        self.outputs.new("VertexInputAttributeNodeSocket", "Attribute")

    def draw_buttons(self, context, layout):
        layout.prop(self, "location_property")
        layout.prop(self, "binding_property")
        layout.prop(self, "format_property")
        layout.prop(self, "offset_property")


vertex_input_rate_values = [
    ("Vertex", "Vertex", "", 0),
    ("Instance", "Instance", "", 1),
]

class VertexInputBindingNode(bpy.types.Node, RenderTreeNode):

    bl_label = 'Vertex Input Binding node'

    binding_property: bpy.props.IntProperty(name="Binding", min=0)
    stride_property: bpy.props.IntProperty(name="Stride", min=1)
    input_rate_property: bpy.props.EnumProperty(name="Input Rate", items=vertex_input_rate_values)

    def init(self, context):
        self.outputs.new("VertexInputBindingNodeSocket", "Binding")

    def draw_buttons(self, context, layout):
        layout.prop(self, "binding_property")
        layout.prop(self, "stride_property")
        layout.prop(self, "input_rate_property")
    

class VertexInputStateNode(bpy.types.Node, RenderTreeNode):

    bl_label = 'Vertex Input State node'

    def init(self, context):
        self.inputs.new("VertexInputBindingNodeSocket", "Bindings")
        self.inputs["Bindings"].link_limit = 0
        self.inputs.new("VertexInputAttributeNodeSocket", "Attributes")
        self.inputs["Attributes"].link_limit = 0

        self.outputs.new("VertexInputStateNodeSocket", "Vertex Input State")


import nodeitems_utils

class PipelineStateNodeCategory(nodeitems_utils.NodeCategory):
    
    @classmethod
    def poll(cls, context):
        return context.space_data.tree_type == 'RenderNodeTree'


pipeline_state_node_categories = [
    PipelineStateNodeCategory('PIPELINE_STATE', "Pipeline State", items=[
        nodeitems_utils.NodeItem("PipelineShaderStageNode"),
        nodeitems_utils.NodeItem("ShaderModuleNode"),
        nodeitems_utils.NodeItem("VertexInputAttributeNode"),
        nodeitems_utils.NodeItem("VertexInputBindingNode"),
        nodeitems_utils.NodeItem("VertexInputStateNode"),
    ]),
]
