import bpy

from .render_node_tree import RenderTreeNode
from .vulkan_enums import cull_modes, format_values, front_face, polygon_modes

class Extent2DNodeSocket(bpy.types.NodeSocket):
    
    bl_label = "Extent 2D node socket"

    def draw(self, context, layout, node, text):
        layout.label(text=text)

    def draw_color(self, context, node):
        return (0.75, 0.75, 0.75, 1.0)

class InputAssemblyStateNodeSocket(bpy.types.NodeSocket):

    bl_label = "Input Assembly State node socket"

    def draw(self, context, layout, node, text):
        layout.label(text=text)

    def draw_color(self, context, node):
        return (0.5, 0.5, 0.0, 1.0)

class Offset2DNodeSocket(bpy.types.NodeSocket):
    
    bl_label = "Offset 2D node socket"

    def draw(self, context, layout, node, text):
        layout.label(text=text)

    def draw_color(self, context, node):
        return (0.25, 0.25, 0.25, 1.0)

class PipelineShaderStageNodeSocket(bpy.types.NodeSocket):
    
    bl_label = "Pipeline Shader Node Socket"

    def draw(self, context, layout, node, text):
        layout.label(text=text)

    def draw_color(self, context, node):
        return (0.0, 1.0, 0.0, 1.0)

class RasterizationStateNodeSocket(bpy.types.NodeSocket):

    bl_label = "Rasterization State node socket"

    def draw(self, context, layout, node, text):
        layout.label(text=text)

    def draw_color(self, context, node):
        return (0.25, 1.0, 0.25, 1.0)

class Rect2DNodeSocket(bpy.types.NodeSocket):

    bl_label = "Scissor node socket"

    def draw(self, context, layout, node, text):
        layout.label(text=text)

    def draw_color(self, context, node):
        return (0.0, 0.25, 0.25, 1.0)

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

class ViewportNodeSocket(bpy.types.NodeSocket):

    bl_label = "Viewport node socket"

    def draw(self, context, layout, node, text):
        layout.label(text=text)

    def draw_color(self, context, node):
        return (0.25, 0.0, 0.25, 1.0)

class ViewportStateNodeSocket(bpy.types.NodeSocket):

    bl_label = "Viewport node socket"

    def draw(self, context, layout, node, text):
        layout.label(text=text)

    def draw_color(self, context, node):
        return (0.1, 0.5, 0.5, 1.0)

topology_values = (
    ("POINT_LIST", "Point List", "", 0),
    ("LINE_LIST", "Line List", "", 1),
    ("LINE_STRIP", "Line Strip", "", 2),
    ("TRIANGLE_LIST", "Triangle List", "", 3),
    ("TRIANGLE_STRIP", "Triangle Strip", "", 4),
    ("TRIANGLE_FAN", "Triangle Fan", "", 5),
    ("LINE_LIST_WITH_ADJACENCY", "Line List With Adjacency", "", 6),
    ("LINE_STRIP_WITH_ADJACENCY", "Line Strip With Adjacency", "", 7),
    ("TRIANGLE_LIST_WITH_ADJACENCY", "Triangle List With Adjacency", "", 8),
    ("TRIANGLE_STRIP_WITH_ADJACENCY", "Triangle Strip With Adjacency", "", 9),
    ("PATCH_LIST", "Patch List", "", 10),
)

class Extent2DNode(bpy.types.Node, RenderTreeNode):

    bl_label = 'Extent 2D node'

    width_property: bpy.props.IntProperty(name="Width", default=800, min=0)
    height_property: bpy.props.IntProperty(name="Height", default=600, min=0)

    def init(self, context):
        
        self.outputs.new("Extent2DNodeSocket", "Extent 2D")

    def draw_buttons(self, context, layout):

        layout.prop(self, "width_property")
        layout.prop(self, "height_property")

class InputAssemblyStateNode(bpy.types.Node, RenderTreeNode):

    bl_label = 'Input Assembly State node'

    topology_property: bpy.props.EnumProperty(name="Topology", items=topology_values)
    primitive_restart_enable_property: bpy.props.BoolProperty(name="Primitive Restart Enable")

    def init(self, context):
        self.outputs.new("InputAssemblyStateNodeSocket", "Input Assembly State")

    def draw_buttons(self, context, layout):
        layout.prop(self, "topology_property")
        layout.prop(self, "primitive_restart_enable_property")


class Offset2DNode(bpy.types.Node, RenderTreeNode):

    bl_label = 'Offset 2D node'

    x_property: bpy.props.IntProperty(name="X", default=0)
    y_property: bpy.props.IntProperty(name="Y", default=0)

    def init(self, context):
        
        self.outputs.new("Offset2DNodeSocket", "Offset 2D")

    def draw_buttons(self, context, layout):

        layout.prop(self, "x_property")
        layout.prop(self, "y_property")

class PipelineShaderStageNode(bpy.types.Node, RenderTreeNode):

    bl_label = 'Pipeline Shader Stage node'

    def init(self, context):
        self.inputs.new("ShaderModuleNodeSocket", "Shader")
        self.outputs.new("PipelineShaderStageNodeSocket", "Stage")

class RasterizationStateNode(bpy.types.Node, RenderTreeNode):

    bl_label = 'Rasterization State node'
    
    depth_clamp_enable_property: bpy.props.BoolProperty(name="Depth Clamp Enable", default=False)
    rasterizer_discard_enable_property: bpy.props.BoolProperty(name="Rasterizer Discard Enable", default=False)
    polygon_mode_property: bpy.props.EnumProperty(name="Polygon Mode", items=polygon_modes, default="FILL")
    cull_mode_property: bpy.props.EnumProperty(name="Cull Mode", items=cull_modes, default={"BACK"}, options={"ANIMATABLE", "ENUM_FLAG"})
    front_face_property: bpy.props.EnumProperty(name="Front Face", items=front_face, default="COUNTER_CLOCKWISE")
    depth_bias_enable_property: bpy.props.BoolProperty(name="Depth Bias Enable", default=False)
    depth_bias_constant_factor_property: bpy.props.FloatProperty(name="Depth Bias Constant Factor", default=0.0)
    depth_bias_clamp_property: bpy.props.FloatProperty(name="Depth Bias Clamp", default=0.0)
    depth_bias_slope_factor_property: bpy.props.FloatProperty(name="Depth Bias Slope Factor", default=0.0)
    line_width_property: bpy.props.FloatProperty(name="Line Width", default=1.0)

    def init(self, context):
        self.outputs.new("RasterizationStateNodeSocket", "State")

    def draw_buttons(self, context, layout):
        layout.prop(self, "depth_clamp_enable_property")
        layout.prop(self, "rasterizer_discard_enable_property")
        layout.prop(self, "polygon_mode_property")
        layout.label(text="Cull Mode")
        layout.prop(self, "cull_mode_property")
        layout.prop(self, "front_face_property")
        layout.prop(self, "depth_bias_enable_property")
        layout.prop(self, "depth_bias_constant_factor_property")
        layout.prop(self, "depth_bias_clamp_property")
        layout.prop(self, "depth_bias_slope_factor_property")
        layout.prop(self, "line_width_property")

class Rect2DNode(bpy.types.Node, RenderTreeNode):

    bl_label = 'Rect2D node'

    def init(self, context):
        self.inputs.new("Offset2DNodeSocket", "Offset")
        self.inputs.new("Extent2DNodeSocket", "Extent")
        
        self.outputs.new("Rect2DNodeSocket", "Scissor")


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

class ViewportNode(bpy.types.Node, RenderTreeNode):

    bl_label = 'Viewport node'

    x_property: bpy.props.FloatProperty(name="X", default=0.0)
    y_property: bpy.props.FloatProperty(name="Y", default=0.0)
    width_property: bpy.props.FloatProperty(name="Width", min=0.00001, default=800.0)
    height_property: bpy.props.FloatProperty(name="Height", min=0.00001, default=600.0)
    minimum_depth_property: bpy.props.FloatProperty(name="Minimum Depth", min=0.0, max=1.0)
    maximum_depth_property: bpy.props.FloatProperty(name="Maximum Depth", min=0.0, max=1.0)

    def init(self, context):
        
        self.outputs.new("ViewportNodeSocket", "Viewport")

    def draw_buttons(self, context, layout):

        layout.prop(self, "x_property")
        layout.prop(self, "y_property")
        layout.prop(self, "width_property")
        layout.prop(self, "height_property")
        layout.prop(self, "minimum_depth_property")
        layout.prop(self, "maximum_depth_property")


class ViewportStateNode(bpy.types.Node, RenderTreeNode):

    bl_label = 'Viewport State node'

    viewport_count_property: bpy.props.IntProperty(name="Viewport Count", min=1)
    dynamic_viewport_property: bpy.props.BoolProperty(name="Dynamic Viewport")
    scissor_count_property: bpy.props.IntProperty(name="Scissor Count", min=1)
    dynamic_scissor_property: bpy.props.BoolProperty(name="Dynamic Scissor")

    def init(self, context):
        self.inputs.new("ViewportNodeSocket", "Viewports")
        self.inputs["Viewports"].link_limit = 0
        self.inputs.new("Rect2DNodeSocket", "Scissors")
        self.inputs["Scissors"].link_limit = 0

        self.outputs.new("ViewportStateNodeSocket", "Viewport State")

    def draw_buttons(self, context, layout):

        layout.prop(self, "dynamic_viewport_property")
        
        if self.dynamic_viewport_property:
            layout.prop(self, "viewport_count_property")

        layout.prop(self, "dynamic_scissor_property")

        if self.dynamic_scissor_property:
            layout.prop(self, "scissor_count_property")

    def update(self):
        
        self.inputs["Viewports"].enabled = not self.dynamic_viewport_property
        self.inputs["Scissors"].enabled = not self.dynamic_scissor_property
        


import nodeitems_utils

class PipelineStateNodeCategory(nodeitems_utils.NodeCategory):
    
    @classmethod
    def poll(cls, context):
        return context.space_data.tree_type == 'RenderNodeTree'


pipeline_state_node_categories = [
    PipelineStateNodeCategory('PIPELINE_STATE', "Pipeline State", items=[
        nodeitems_utils.NodeItem("Extent2DNode"),
        nodeitems_utils.NodeItem("InputAssemblyStateNode"),
        nodeitems_utils.NodeItem("Offset2DNode"),
        nodeitems_utils.NodeItem("PipelineShaderStageNode"),
        nodeitems_utils.NodeItem("RasterizationStateNode"),
        nodeitems_utils.NodeItem("Rect2DNode"),
        nodeitems_utils.NodeItem("ShaderModuleNode"),
        nodeitems_utils.NodeItem("VertexInputAttributeNode"),
        nodeitems_utils.NodeItem("VertexInputBindingNode"),
        nodeitems_utils.NodeItem("VertexInputStateNode"),
        nodeitems_utils.NodeItem("ViewportNode"),
        nodeitems_utils.NodeItem("ViewportStateNode"),
    ]),
]
