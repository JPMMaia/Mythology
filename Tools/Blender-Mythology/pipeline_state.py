import bpy

from .render_node_tree import RenderTreeNode
from .render_pass import RenderPassNodeSocket, SubpassNodeSocket
from .vulkan_enums import border_color_values, blend_factor_values, blend_operation_values, color_component_flag_values, compare_operation_values, cull_modes, descriptor_type_values, dynamic_state_values, filter_values, format_values, front_face, logic_operation_values, polygon_modes, sampler_address_move_values, sampler_mipmap_mode_values, shader_stage_flag_values, stencil_operation_values


class ColorBlendAttachmentStateNodeSocket(bpy.types.NodeSocket):
    
    bl_label = "Color Blender Attachment State node socket"

    def draw(self, context, layout, node, text):
        layout.label(text=text)

    def draw_color(self, context, node):
        return (0.8, 0.2, 0.6, 1.0)

class ColorBlendStateNodeSocket(bpy.types.NodeSocket):
    
    bl_label = "Color Blender State node socket"

    def draw(self, context, layout, node, text):
        layout.label(text=text)

    def draw_color(self, context, node):
        return (0.2, 0.6, 0.8, 1.0)

class DepthStencilStateNodeSocket(bpy.types.NodeSocket):
    
    bl_label = "Depth Stencil State node socket"

    def draw(self, context, layout, node, text):
        layout.label(text=text)

    def draw_color(self, context, node):
        return (0.0, 0.75, 0.25, 1.0)


class DescriptorSetLayoutBindingNodeSocket(bpy.types.NodeSocket):
    
    bl_label = "Descriptor Set Layout Binding node socket"

    def draw(self, context, layout, node, text):
        layout.label(text=text)

    def draw_color(self, context, node):
        return (1.0, 0.2, 0.4, 1.0)

class DescriptorSetLayoutNodeSocket(bpy.types.NodeSocket):
    
    bl_label = "Descriptor Set Layout node socket"

    def draw(self, context, layout, node, text):
        layout.label(text=text)

    def draw_color(self, context, node):
        return (0.2, 0.4, 0.6, 1.0)

class DynamicStateNodeSocket(bpy.types.NodeSocket):
    
    bl_label = "Dynamic State node socket"

    def draw(self, context, layout, node, text):
        layout.label(text=text)

    def draw_color(self, context, node):
        return (1.0, 0.67, 0.33, 1.0)


class InputAssemblyStateNodeSocket(bpy.types.NodeSocket):

    bl_label = "Input Assembly State node socket"

    def draw(self, context, layout, node, text):
        layout.label(text=text)

    def draw_color(self, context, node):
        return (0.5, 0.5, 0.0, 1.0)


class PipelineDynamicStateNodeSocket(bpy.types.NodeSocket):
    
    bl_label = "Pipeline Dynamic State node socket"

    def draw(self, context, layout, node, text):
        layout.label(text=text)

    def draw_color(self, context, node):
        return (0.67, 0.33, 1.0, 1.0)

class PipelineLayoutNodeSocket(bpy.types.NodeSocket):
    
    bl_label = "Pipeline Layout node socket"

    def draw(self, context, layout, node, text):
        layout.label(text=text)

    def draw_color(self, context, node):
        return (0.7, 0.6, 0.2, 1.0)


class PipelineNodeSocket(bpy.types.NodeSocket):
    
    bl_label = "Pipeline Node Socket"

    def draw(self, context, layout, node, text):
        layout.label(text=text)

    def draw_color(self, context, node):
        return (0.33, 0.25, 0.8, 1.0)

class PipelineShaderStageNodeSocket(bpy.types.NodeSocket):
    
    bl_label = "Pipeline Shader Node Socket"

    def draw(self, context, layout, node, text):
        layout.label(text=text)

    def draw_color(self, context, node):
        return (0.0, 1.0, 0.0, 1.0)

class PushConstantRangeNodeSocket(bpy.types.NodeSocket):
    
    bl_label = "Push Constant Range node socket"

    def draw(self, context, layout, node, text):
        layout.label(text=text)

    def draw_color(self, context, node):
        return (0.5, 0.2, 0.9, 1.0)

class RasterizationStateNodeSocket(bpy.types.NodeSocket):

    bl_label = "Rasterization State node socket"

    def draw(self, context, layout, node, text):
        layout.label(text=text)

    def draw_color(self, context, node):
        return (0.25, 1.0, 0.25, 1.0)


class SamplerNodeSocket(bpy.types.NodeSocket):

    bl_label = "Sampler node socket"

    def draw(self, context, layout, node, text):
        layout.label(text=text)

    def draw_color(self, context, node):
        return (0.2, 0.2, 0.2, 1.0)

class ShaderModuleNodeSocket(bpy.types.NodeSocket):
    
    bl_label = "Shader Module Node Socket"

    def draw(self, context, layout, node, text):
        layout.label(text=text)

    def draw_color(self, context, node):
        return (1.0, 0.0, 0.0, 1.0)

class StencilOperationStateNodeSocket(bpy.types.NodeSocket):
    
    bl_label = "Stencil Operation State Node Socket"

    def draw(self, context, layout, node, text):
        layout.label(text=text)

    def draw_color(self, context, node):
        return (0.25, 0.5, 0.75, 1.0)

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

class ColorBlendAttachmentStateNode(bpy.types.Node, RenderTreeNode):

    bl_label = 'Color Blend Attachment State node'

    blend_enable_property: bpy.props.BoolProperty(name="Blend Enable", default=False)
    source_color_blend_factor_property: bpy.props.EnumProperty(name="Source Color Blend Factor", items=blend_factor_values, default="ZERO")
    destination_color_blend_factor_property: bpy.props.EnumProperty(name="Destination Color Blend Factor", items=blend_factor_values, default="ZERO")
    color_blend_operation_property: bpy.props.EnumProperty(name="Color Blend Operation", items=blend_operation_values, default="ADD")
    source_alpha_blend_factor_property: bpy.props.EnumProperty(name="Source Alpha Blend Factor", items=blend_factor_values, default="ZERO")
    destination_alpha_blend_factor_property: bpy.props.EnumProperty(name="Destination Alpha Blend Factor", items=blend_factor_values, default="ZERO")
    alpha_blend_operation_property: bpy.props.EnumProperty(name="Alpha Blend Operation", items=blend_operation_values, default="ADD")
    color_write_mask_property: bpy.props.EnumProperty(name="Color Write Mask", items=color_component_flag_values, default={"R", "G", "B", "A"}, options={"ANIMATABLE", "ENUM_FLAG"})

    def init(self, context):

        self.outputs.new("ColorBlendAttachmentStateNodeSocket", "Attachment State")

    def draw_buttons(self, context, layout):

        layout.prop(self, "blend_enable_property")
        layout.prop(self, "source_color_blend_factor_property")
        layout.prop(self, "destination_color_blend_factor_property")
        layout.prop(self, "color_blend_operation_property")
        layout.prop(self, "source_alpha_blend_factor_property")
        layout.prop(self, "destination_alpha_blend_factor_property")
        layout.prop(self, "alpha_blend_operation_property")
        layout.prop(self, "color_write_mask_property")

class ColorBlendStateNode(bpy.types.Node, RenderTreeNode):

    bl_label = 'Color Blend State node'

    logic_operation_enable_property: bpy.props.BoolProperty(name="Logic Operation Enable", default=False)
    logic_operation_property: bpy.props.EnumProperty(name="Logic Operation", items=logic_operation_values, default="CLEAR")
    blend_constants_property: bpy.props.FloatVectorProperty(name="Blend Constants", size=4, default=(0.0, 0.0, 0.0, 0.0))

    def init(self, context):
        
        self.inputs.new("ColorBlendAttachmentStateNodeSocket", "Attachments")
        self.inputs["Attachments"].link_limit = 0

        self.outputs.new("ColorBlendStateNodeSocket", "State")

    def draw_buttons(self, context, layout):

        layout.prop(self, "logic_operation_enable_property")
        layout.prop(self, "logic_operation_property")
        layout.prop(self, "blend_constants_property")

class DepthStencilStateNode(bpy.types.Node, RenderTreeNode):

    bl_label = 'Depth Stencil State node'

    depth_test_enable_property: bpy.props.BoolProperty(name="Depth Test Enable", default=False)
    depth_write_enable_property: bpy.props.BoolProperty(name="Depth Write Enable", default=False)
    compare_operation_property: bpy.props.EnumProperty(name="Compare Operation", items=compare_operation_values, default="NEVER")
    depth_bounds_test_enable_property: bpy.props.BoolProperty(name="Depth Bounds Test Enable", default=False)
    stencil_test_enable_property: bpy.props.BoolProperty(name="Stencil Test Enable", default=False)
    min_depth_bounds_property: bpy.props.FloatProperty(name="Min Depth Bounds", default=0.0)
    max_depth_bounds_property: bpy.props.FloatProperty(name="Max Depth Bounds", default=1.0)

    def init(self, context):
        
        self.inputs.new("StencilOperationStateNodeSocket", "Front Stencil State")
        self.inputs.new("StencilOperationStateNodeSocket", "Back Stencil State")

        self.outputs.new("DepthStencilStateNodeSocket", "State")

    def draw_buttons(self, context, layout):

        layout.prop(self, "depth_test_enable_property")
        layout.prop(self, "depth_write_enable_property")
        layout.prop(self, "compare_operation_property")
        layout.prop(self, "depth_bounds_test_enable_property")
        layout.prop(self, "stencil_test_enable_property")
        layout.prop(self, "min_depth_bounds_property")
        layout.prop(self, "max_depth_bounds_property")

class DescriptorSetLayoutBindingNode(bpy.types.Node, RenderTreeNode):

    bl_label = 'Descriptor Set Layout Binding node'

    binding_property: bpy.props.IntProperty(name="Binding", min=0)
    descriptor_type_property: bpy.props.EnumProperty(name="Descriptor Type", items=descriptor_type_values)
    descriptor_count_property: bpy.props.IntProperty(name="Descriptor Count", min=1)
    stage_flags_property: bpy.props.EnumProperty(name="Stage Flags", items=shader_stage_flag_values, options={"ANIMATABLE", "ENUM_FLAG"})

    def init(self, context):

        self.inputs.new("SamplerNodeSocket", "Immutable Samplers")

        self.outputs.new("DescriptorSetLayoutBindingNodeSocket", "Descriptor Set Layout Binding")

    def draw_buttons(self, context, layout):

        layout.prop(self, "binding_property")
        layout.prop(self, "descriptor_type_property")
        layout.prop(self, "descriptor_count_property")
        layout.prop(self, "stage_flags_property")

class DescriptorSetLayoutNode(bpy.types.Node, RenderTreeNode):

    bl_label = 'Descriptor Set Layout node'

    def init(self, context):
        
        self.inputs.new("DescriptorSetLayoutBindingNodeSocket", "Bindings")
        self.inputs["Bindings"].link_limit = 0

        self.outputs.new("DescriptorSetLayoutNodeSocket", "Descriptor Set Layout")


class DynamicStateNode(bpy.types.Node, RenderTreeNode):

    bl_label = 'Dynamic State node'

    dynamic_state_property: bpy.props.EnumProperty(name="Value", items=dynamic_state_values)

    def init(self, context):

        self.outputs.new("DynamicStateNodeSocket", "Value")

    def draw_buttons(self, context, layout):

        layout.prop(self, "dynamic_state_property")

class GraphicsPipelineStateNode(bpy.types.Node, RenderTreeNode):

    bl_label = 'Graphics Pipeline State node'

    name_property: bpy.props.StringProperty(name="Name")

    def init(self, context):
        self.inputs.new("PipelineShaderStageNodeSocket", "Stages")
        self.inputs["Stages"].link_limit = 0
        self.inputs.new("VertexInputStateNodeSocket", "Vertex Input State")
        self.inputs.new("InputAssemblyStateNodeSocket", "Input Assembly State")
        self.inputs.new("ViewportStateNodeSocket", "Viewport State")
        self.inputs.new("RasterizationStateNodeSocket", "Rasterization State")
        self.inputs.new("DepthStencilStateNodeSocket", "Depth Stencil State")
        self.inputs.new("ColorBlendStateNodeSocket", "Color Blend State")
        self.inputs.new("PipelineDynamicStateNodeSocket", "Dynamic State")
        self.inputs.new("PipelineLayoutNodeSocket", "Pipeline Layout") # TODO
        self.inputs.new("RenderPassNodeSocket", "Render Pass")
        self.inputs.new("SubpassNodeSocket", "Subpass")

        self.outputs.new("PipelineNodeSocket", "Pipeline")

    def draw_buttons(self, context, layout):
        layout.prop(self, "name_property")


class InputAssemblyStateNode(bpy.types.Node, RenderTreeNode):

    bl_label = 'Input Assembly State node'

    topology_property: bpy.props.EnumProperty(name="Topology", items=topology_values)
    primitive_restart_enable_property: bpy.props.BoolProperty(name="Primitive Restart Enable")

    def init(self, context):
        self.outputs.new("InputAssemblyStateNodeSocket", "Input Assembly State")

    def draw_buttons(self, context, layout):
        layout.prop(self, "topology_property")
        layout.prop(self, "primitive_restart_enable_property")


class PipelineDynamicStateNode(bpy.types.Node, RenderTreeNode):

    bl_label = 'Pipeline Dynamic State node'

    def init(self, context):
        self.inputs.new("DynamicStateNodeSocket", "Dynamic States")
        self.inputs["Dynamic States"].link_limit = 0

        self.outputs.new("PipelineDynamicStateNodeSocket", "State")

class PipelineLayoutNode(bpy.types.Node, RenderTreeNode):

    bl_label = 'Pipeline Layout node'

    def init(self, context):
        self.inputs.new("DescriptorSetLayoutNodeSocket", "Descriptor Set Layouts")
        self.inputs.new("PushConstantRangeNodeSocket", "Push Constant Ranges")
        
        self.outputs.new("PipelineLayoutNodeSocket", "Pipeline Layout")


class PipelineShaderStageNode(bpy.types.Node, RenderTreeNode):

    bl_label = 'Pipeline Shader Stage node'

    def init(self, context):
        self.inputs.new("ShaderModuleNodeSocket", "Shader")
        self.outputs.new("PipelineShaderStageNodeSocket", "Stage")

class PushConstantRangeNode(bpy.types.Node, RenderTreeNode):

    bl_label = 'Push Constant Range node'

    stage_flags_property: bpy.props.EnumProperty(name="Stage Flags", items=shader_stage_flag_values, options={"ANIMATABLE", "ENUM_FLAG"})
    offset_property: bpy.props.IntProperty(name="Offset in bytes", default=0)
    size_property: bpy.props.IntProperty(name="Size in bytes", default=0)

    def init(self, context):
        
        self.outputs.new("PushConstantRangeNodeSocket", "Push Constant Range")

    def draw_buttons(self, context, layout):

        layout.label(text="Stages")
        layout.prop(self, "stage_flags_property")
        layout.prop(self, "offset_property")
        layout.prop(self, "size_property")

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


class SamplerNode(bpy.types.Node, RenderTreeNode):

    bl_label = 'Sampler node'

    mag_filter_property: bpy.props.EnumProperty(name="Mag Filter", items=filter_values)
    min_filter_property: bpy.props.EnumProperty(name="Min Filter", items=filter_values)
    mipmap_mode_property: bpy.props.EnumProperty(name="Mipmap Mode", items=sampler_mipmap_mode_values)
    address_mode_u_property: bpy.props.EnumProperty(name="Address Mode U", items=sampler_address_move_values)
    address_mode_v_property: bpy.props.EnumProperty(name="Address Mode V", items=sampler_address_move_values)
    address_mode_w_property: bpy.props.EnumProperty(name="Address Mode W", items=sampler_address_move_values)
    mip_lod_bias_property: bpy.props.FloatProperty(name="Mip Lod Bias", default=0.0)
    anisotropy_enable_property: bpy.props.BoolProperty(name="Anisotropy Enable")
    max_anisotropy_property: bpy.props.FloatProperty(name="Max Anisotropy", default=0.0)
    compare_enable_property: bpy.props.BoolProperty(name="Compare Enable")
    compare_operation_property: bpy.props.EnumProperty(name="Compare Operation", items=compare_operation_values)
    min_lod_property: bpy.props.FloatProperty(name="Min Lod", default=0.0)
    max_lod_property: bpy.props.FloatProperty(name="Max Lod", default=0.0)
    border_color_property: bpy.props.EnumProperty(name="Border Color", items=border_color_values)
    unnormalized_coordinates_property: bpy.props.BoolProperty(name="Unnormalized Coordinates")
    
    
    def init(self, context):
        self.outputs.new("SamplerNodeSocket", "Sampler")

    def draw_buttons(self, context, layout):
        layout.prop(self, "mag_filter_property")
        layout.prop(self, "min_filter_property")
        layout.prop(self, "mipmap_mode_property")
        layout.prop(self, "address_mode_u_property")
        layout.prop(self, "address_mode_v_property")
        layout.prop(self, "address_mode_w_property")
        layout.prop(self, "mip_lod_bias_property")
        layout.prop(self, "anisotropy_enable_property")
        layout.prop(self, "max_anisotropy_property")
        layout.prop(self, "compare_enable_property")
        layout.prop(self, "compare_operation_property")
        layout.prop(self, "min_lod_property")
        layout.prop(self, "max_lod_property")
        layout.prop(self, "border_color_property")
        layout.prop(self, "unnormalized_coordinates_property")


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

    shader_file_property: bpy.props.StringProperty(name="File", subtype="FILE_PATH")
    language_property: bpy.props.EnumProperty(items=[("HLSL", "HLSL", "", 0)])
    shader_type_property: bpy.props.EnumProperty(name="Type", items=shader_type_values)
    shader_model_property: bpy.props.StringProperty(name="Model", default="6_5")
    entry_point_property: bpy.props.StringProperty(name="Entry point", default="main")
    additional_compile_flags_property: bpy.props.StringProperty(name="Compile flags", description="Additional compile flags")

    def init(self, context):
        self.outputs.new("ShaderModuleNodeSocket", "Shader")

    def draw_buttons(self, context, layout):
        layout.prop(self, "shader_file_property")
        layout.prop(self, "language_property")
        layout.prop(self, "shader_type_property")
        layout.prop(self, "shader_model_property")
        layout.prop(self, "entry_point_property")
        layout.prop(self, "additional_compile_flags_property")

class StencilOperationStateNode(bpy.types.Node, RenderTreeNode):

    bl_label = 'Stencil Operation State node'

    fail_operation_property: bpy.props.EnumProperty(name="Fail Operation", items=stencil_operation_values, default="KEEP")
    pass_operation_property: bpy.props.EnumProperty(name="Pass Operation", items=stencil_operation_values, default="KEEP")
    depth_fail_operation_property: bpy.props.EnumProperty(name="Depth Fail Operation", items=stencil_operation_values, default="KEEP")
    compare_operation_property: bpy.props.EnumProperty(name="Compare Operation", items=compare_operation_values, default="NEVER")
    compare_mask_property: bpy.props.IntProperty(name="Compare Mask", default=2**31-1, min=0, max=2**31-1)
    write_mask_property: bpy.props.IntProperty(name="Write Mask", default=2**31-1, min=0, max=2**31-1)
    reference_property: bpy.props.IntProperty(name="Reference", default=0, min=0)

    def init(self, context):
        
        self.outputs.new("StencilOperationStateNodeSocket", "State")

    def draw_buttons(self, context, layout):

        layout.prop(self, "fail_operation_property")
        layout.prop(self, "pass_operation_property")
        layout.prop(self, "depth_fail_operation_property")
        layout.prop(self, "compare_operation_property")
        layout.prop(self, "compare_mask_property")
        layout.prop(self, "write_mask_property")
        layout.prop(self, "reference_property")


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
        

import nodeitems_utils

class PipelineStateNodeCategory(nodeitems_utils.NodeCategory):
    
    @classmethod
    def poll(cls, context):
        return context.space_data.tree_type == 'RenderNodeTree'


pipeline_state_node_categories = [
    PipelineStateNodeCategory('PIPELINE_STATE', "Pipeline State", items=[
        nodeitems_utils.NodeItem("ColorBlendAttachmentStateNode"),
        nodeitems_utils.NodeItem("ColorBlendStateNode"),
        nodeitems_utils.NodeItem("DepthStencilStateNode"),
        nodeitems_utils.NodeItem("DescriptorSetLayoutBindingNode"),
        nodeitems_utils.NodeItem("DescriptorSetLayoutNode"),
        nodeitems_utils.NodeItem("DynamicStateNode"),
        nodeitems_utils.NodeItem("GraphicsPipelineStateNode"),
        nodeitems_utils.NodeItem("InputAssemblyStateNode"),
        nodeitems_utils.NodeItem("PipelineDynamicStateNode"),
        nodeitems_utils.NodeItem("PipelineLayoutNode"),
        nodeitems_utils.NodeItem("PipelineShaderStageNode"),
        nodeitems_utils.NodeItem("PushConstantRangeNode"),
        nodeitems_utils.NodeItem("RasterizationStateNode"),
        nodeitems_utils.NodeItem("SamplerNode"),
        nodeitems_utils.NodeItem("ShaderModuleNode"),
        nodeitems_utils.NodeItem("StencilOperationStateNode"),
        nodeitems_utils.NodeItem("VertexInputAttributeNode"),
        nodeitems_utils.NodeItem("VertexInputBindingNode"),
        nodeitems_utils.NodeItem("VertexInputStateNode"),
        nodeitems_utils.NodeItem("ViewportNode"),
        nodeitems_utils.NodeItem("ViewportStateNode"),
    ]),
]

import typing

from .common import create_rect_2d_json
from .render_pass import RenderPassNode, SubpassNode

JSONType = typing.Union[str, int, float, bool, None, typing.Dict[str, typing.Any], typing.List[typing.Any]]

def create_sampler_json(
    node: bpy.types.Node
) -> JSONType:

    return {
        "mag_filter": node.get("mag_filter_property", 0),
        "min_filter": node.get("min_filter_property", 0),
        "mipmap_mode": node.get("mipmap_mode_property", 0),
        "address_mode_u": node.get("address_mode_u_property", 0),
        "address_mode_v": node.get("address_mode_v_property", 0),
        "address_mode_w": node.get("address_mode_w_property", 0),
        "mip_lod_bias": node.get("mip_lod_bias_property", 0.0),
        "anisotropy_enable": node.get("anisotropy_enable_property", False),
        "max_anisotropy": node.get("max_anisotropy_property", 0.0),
        "compare_enable": node.get("compare_enable_property", False),
        "compare_operation": node.get("compare_operation_property", 0),
        "min_lod": node.get("min_lod_property", 0.0),
        "max_lod": node.get("max_lod_property", 0.0),
        "border_color": node.get("border_color_property", 0),
        "unnormalized_coordinates": node.get("unnormalized_coordinates_property", False),
    }


def create_samplers_json(
    nodes: typing.List[bpy.types.Node]
) -> typing.Tuple[typing.List[PipelineLayoutNode], JSONType]:

    sampler_nodes = [node
                     for node in nodes
                     if node.bl_idname == "SamplerNode"]

    json = [create_sampler_json(sampler_node)
            for sampler_node in sampler_nodes]

    return (sampler_nodes, json)


def create_descriptor_set_layouts_json(
    nodes: typing.List[bpy.types.Node],
    samplers: typing.Tuple[typing.List[bpy.types.Node], JSONType],
) -> typing.Tuple[typing.List[DescriptorSetLayoutNode], JSONType]:

    descriptor_set_layout_nodes = [node
                                   for node in nodes
                                   if node.bl_idname == "DescriptorSetLayoutNode"]

    json = [
        {
            "bindings": [
                {
                    "binding": link.from_node.get("binding_property", 0),
                    "descriptor_type": link.from_node.get("descriptor_type_property", 0),
                    "descriptor_count": link.from_node.get("descriptor_count_property", 1),
                    "stage_flags_property": link.from_node.get("stage_flags_property_property", 0),
                    "immutable_samplers": [samplers[0].index(sampler_link.from_node)
                                           for sampler_link in link.from_node.inputs["Immutable Samplers"].links],
                }
                for link in node.inputs["Bindings"].links
            ],
        }
        for node in descriptor_set_layout_nodes
    ]

    return (descriptor_set_layout_nodes, json)

def create_push_constant_range_json(
    node: PushConstantRangeNode
) -> JSONType:

    return {
        "stage_flags": node.get("stage_flags_property", 0),
        "offset": node.get("offset_property", 0),
        "size": node.get("size_property", 0),
    }

def create_pipeline_layouts_json(
    nodes: typing.List[bpy.types.Node],
    descriptor_set_layouts: typing.Tuple[typing.List[DescriptorSetLayoutNode], JSONType]
) -> typing.Tuple[typing.List[PipelineLayoutNode], JSONType]:

    pipeline_layout_nodes = [node
                             for node in nodes
                             if node.bl_idname == "PipelineLayoutNode"]

    json = [
        {
            "descriptor_set_layouts": [descriptor_set_layouts[0].index(link.from_node)
                                       for link in node.inputs["Descriptor Set Layouts"].links],
            "push_constant_ranges": [create_push_constant_range_json(link.from_node)
                                     for link in node.inputs["Descriptor Set Layouts"].links],
        }
        for node in pipeline_layout_nodes
    ]

    return (pipeline_layout_nodes, json)

def shader_module_to_json(
    nodes: typing.List[bpy.types.Node]
) -> typing.Tuple[typing.List[ShaderModuleNode], JSONType]:

    shader_module_nodes = [node
                           for node in nodes
                           if node.bl_idname == "ShaderModuleNode"]

    json = [
        {
            "file": shader_module_node.shader_file_property,
            "language": shader_module_node.language_property,
            "type": shader_module_node.shader_type_property,
            "shader_model": shader_module_node.shader_model_property,
            "entry_point": shader_module_node.entry_point_property,
            "additional_compile_flags": shader_module_node.additional_compile_flags_property,
        }
        for shader_module_node in shader_module_nodes
    ]

    return (shader_module_nodes, json)

def create_shader_stages_json(
    pipeline_shader_stage_node_socket: PipelineShaderStageNodeSocket, 
    shader_modules: typing.Tuple[typing.List[ShaderModuleNode], JSONType]
) -> JSONType:

    assert len(pipeline_shader_stage_node_socket.links) > 0

    shader_stage_nodes = [link.from_node
                          for link in pipeline_shader_stage_node_socket.links]
                                   
    assert all(len(shader_stage_node.inputs['Shader'].links) == 1
               for shader_stage_node in shader_stage_nodes)

    shader_module_nodes = [shader_stage_node.inputs['Shader'].links[0].from_node
                           for shader_stage_node in shader_stage_nodes]
    
    shader_module_indices = [next(index 
                                  for index, node in enumerate(shader_modules[0])
                                  if node == shader_module_node)
                             for shader_module_node in shader_module_nodes]

    return [
        {
            "shader": shader_module_index
        }
        for shader_module_index in shader_module_indices
    ]
    

def create_vertex_input_attribute_json(
    node: VertexInputAttributeNode
) -> JSONType:

    return {
        "location": node.get('location_property', 0),
        "binding": node.get('binding_property', 0),
        "format": node.get('format_property', 0),
        "offset": node.get('offset_property', 0),
    }

def create_vertex_input_binding_json(
    node: VertexInputBindingNode
) -> JSONType:

    return {
        "binding": node.get('binding_property', 0),
        "stride": node.get('stride_property', 0),
        "input_rate": node.get('input_rate_property', 0),
    }

def create_vertex_input_state_json(
    node_socket: VertexInputStateNodeSocket
) -> JSONType:

    if len(node_socket.links) > 0:
        assert len(node_socket.links) == 1

        state_node = node_socket.links[0].from_node

        return {
            "bindings": [create_vertex_input_binding_json(link.from_node)
                         for link in state_node.inputs["Bindings"].links],
            "attributes": [create_vertex_input_attribute_json(link.from_node)
                           for link in state_node.inputs["Attributes"].links]
        }

    else:
        return {}

def create_input_assembly_state_json(
    node_socket: InputAssemblyStateNodeSocket
) -> JSONType:

    if len(node_socket.links) > 0:
        assert len(node_socket.links) == 1

        state_node = node_socket.links[0].from_node

        return {
            "topology": state_node.get('topology_property', 0),
            "primitive_restart_enable": state_node.primitive_restart_enable_property,
        }

    else:
        return {}

def create_viewport_json(
    node: ViewportNode
) -> JSONType:

    return {
        "x": node.get("x_property", 0),
        "y": node.get("y_property", 0),
        "width": node.get("width_property", 800),
        "height": node.get("height_property", 600),
        "minimum": node.get("minimum_property", 0.0),
        "maximum": node.get("maximum_property", 1.0),
    }

def create_viewport_state_json(
    node_socket: ViewportStateNodeSocket
) -> JSONType:

    if len(node_socket.links) > 0:
        assert len(node_socket.links) == 1

        state_node = node_socket.links[0].from_node

        return {
            "viewport_count": state_node.viewport_count_property if state_node.dynamic_viewport_property else len(state_node.inputs["Viewports"].links),
            "scissor_count": state_node.scissor_count_property if state_node.dynamic_scissor_property else len(state_node.inputs["Scissors"].links),
            "viewports": [create_viewport_json(link.from_node)
                          for link in state_node.inputs["Viewports"].links],
            "scissors": [create_rect_2d_json(link.from_node)
                         for link in state_node.inputs["Scissors"].links]
        }

    else:
        return {}

def create_rasterization_state_json(
    node_socket: RasterizationStateNodeSocket
) -> JSONType:

    if len(node_socket.links) > 0:
        assert len(node_socket.links) == 1

        state_node = node_socket.links[0].from_node

        return {
            "depth_clamp_enable": state_node.get("depth_clamp_enable_property", False),
            "rasterizer_discard_enable": state_node.get("rasterizer_discard_enable_property", False),
            "polygon_mode": state_node.get("polygon_mode_property", 0),
            "cull_mode": state_node.get("cull_mode_property", 0),
            "front_face": state_node.get("front_face_property", 0),
            "depth_bias_enable": state_node.get("depth_bias_enable_property", False),
            "depth_bias_constant_factor": state_node.get("depth_bias_constant_factor_property", 0.0),
            "depth_bias_clamp": state_node.get("depth_bias_clamp_property", 0.0),
            "depth_bias_slope_factor": state_node.get("depth_bias_slope_factor_property", 0.0),
            "line_width_factor": state_node.get("line_width_factor_property", 1.0),
        }

    else:
        return {}

def create_stencil_operation_state_json(
    node_socket: StencilOperationStateNodeSocket
) -> JSONType:

    if len(node_socket.links) > 0:
        assert len(node_socket.links) == 1

        state_node = node_socket.links[0].from_node

        return {
            "fail_operation": state_node.get("fail_operation_property", 0),
            "pass_operation": state_node.get("pass_operation_property", 0),
            "depth_fail_operation": state_node.get("depth_fail_operation_property", 0),
            "compare_operation": state_node.get("compare_operation_property", 0),
            "compare_mask": state_node.get("compare_mask_property", 2147483647),
            "write_mask": state_node.get("write_mask_property", 2147483647),
            "reference": state_node.get("reference_property", 0),
        }

    else:
        return {}

def create_depth_stencil_state_json(
    node_socket: DepthStencilStateNodeSocket
) -> JSONType:

    if len(node_socket.links) > 0:
        assert len(node_socket.links) == 1

        state_node = node_socket.links[0].from_node

        return {
            "depth_test_enable": state_node.get("depth_test_enable_property", False),
            "depth_write_enable": state_node.get("depth_write_enable_property", False),
            "compare_operation": state_node.get("compare_operation_property", 0),
            "depth_bounds_test_enable": state_node.get("depth_bounds_test_enable_property", False),
            "stencil_test_enable": state_node.get("stencil_test_enable_property", False),
            "min_depth_bounds": state_node.get("min_depth_bounds_property", 0.0),
            "max_depth_bounds": state_node.get("max_depth_bounds_property", 1.0),
            "front_stencil_state": create_stencil_operation_state_json(state_node.inputs["Front Stencil State"]),
            "back_stencil_state": create_stencil_operation_state_json(state_node.inputs["Back Stencil State"]),
        }

    else:
        return {}

def create_color_blend_attachment_state_json(
    node: ColorBlendAttachmentStateNode
) -> JSONType:

    return {
        "blend_enable": node.get("blend_enable_property", False),
        "source_color_blend_factor": node.get("source_color_blend_factor_property", 0),
        "destination_color_blend_factor": node.get("destination_color_blend_factor_property", 0),
        "color_blend_operation": node.get("color_blend_operation_property", 0),
        "source_alpha_blend_factor": node.get("source_alpha_blend_factor_property", 0),
        "destination_alpha_blend_factor": node.get("destination_alpha_blend_factor_property", 0),
        "alpha_blend_operation": node.get("alpha_blend_operation_property", 0),
    }

def create_color_blend_state_json(
    node_socket: ColorBlendStateNodeSocket
) -> JSONType:

    if len(node_socket.links) > 0:
        assert len(node_socket.links) == 1

        state_node = node_socket.links[0].from_node

        return {
            "logic_operation_enable": state_node.get("logic_operation_enable_property", False),
            "logic_operation": state_node.get("logic_operation_property", 0),
            "blend_constants": [value for value in state_node.get("blend_constants_property", [0.0, 0.0, 0.0, 0.0])],
            "attachments": [create_color_blend_attachment_state_json(link.from_node)
                            for link in state_node.inputs["Attachments"].links],
        }

    else:
        return {}

def create_pipeline_dynamic_state_json(
    node_socket: PipelineDynamicStateNodeSocket
) -> JSONType:

    if len(node_socket.links) > 0:
        assert len(node_socket.links) == 1

        state_node = node_socket.links[0].from_node

        return {
            "dynamic_states": state_node.get("dynamic_state_property", 0),
        }

    else:
        return {}


def pipeline_state_to_json(
    nodes: typing.List[bpy.types.Node],
    render_passes: typing.Tuple[typing.List[RenderPassNode], typing.List[typing.List[SubpassNode]], JSONType],
    shader_modules: typing.Tuple[typing.List[ShaderModuleNode], JSONType],
    pipeline_layouts: typing.Tuple[typing.List[PipelineLayoutNode], JSONType]
) -> typing.Tuple[typing.List[GraphicsPipelineStateNode], JSONType]:
    
    pipeline_state_nodes = [node
                            for node in nodes
                            if node.bl_idname == "GraphicsPipelineStateNode"]

    names = [pipeline_state.name_property
             for pipeline_state in pipeline_state_nodes]

    stages_per_pipeline_state = [create_shader_stages_json(pipeline_state.inputs['Stages'], shader_modules)
                                 for pipeline_state in pipeline_state_nodes]

    render_pass_indices = [render_passes[0].index(pipeline_state.inputs["Render Pass"].links[0].from_node)
                           for pipeline_state in pipeline_state_nodes]
    subpass_indices = [render_passes[1][render_pass_index].index(pipeline_state.inputs["Subpass"].links[0].from_node)
                       for (pipeline_state, render_pass_index) in zip(pipeline_state_nodes, render_pass_indices)]

    json = [
        {
            "name": name,
            "stages": stages,
            "vertex_input_state": create_vertex_input_state_json(pipeline_state.inputs["Vertex Input State"]),
            "input_assembly_state": create_input_assembly_state_json(pipeline_state.inputs["Input Assembly State"]),
            "viewport_state": create_viewport_state_json(pipeline_state.inputs["Viewport State"]),
            "rasterization_state": create_rasterization_state_json(pipeline_state.inputs["Rasterization State"]),
            "depth_stencil_state": create_depth_stencil_state_json(pipeline_state.inputs["Depth Stencil State"]),
            "color_blend_state": create_color_blend_state_json(pipeline_state.inputs["Color Blend State"]),
            "dynamic_state": create_pipeline_dynamic_state_json(pipeline_state.inputs["Dynamic State"]),
            "pipeline_layout": pipeline_layouts[0].index(pipeline_state.inputs["Pipeline Layout"].links[0].from_node),
            "render_pass": render_pass_index,
            "subpass": subpass_index,
        }
        for (pipeline_state, name, stages, render_pass_index, subpass_index) in zip(pipeline_state_nodes, names, stages_per_pipeline_state, render_pass_indices, subpass_indices)
    ]

    return (pipeline_state_nodes, json)

    
