import bpy
import nodeitems_utils

from .pipeline_state import PipelineLibraryNodeSocket
from .render_node_tree import RenderTreeNode


class RayTracingNodeCategory(nodeitems_utils.NodeCategory):
    @classmethod
    def poll(cls, context):
        return context.space_data.tree_type == "RenderNodeTree"


ray_tracing_node_categories = [
    RayTracingNodeCategory(
        "RAY_TRACING",
        "Ray Tracing",
        items=[
            nodeitems_utils.NodeItem("RayTracingPipelineInterfaceNode"),
            nodeitems_utils.NodeItem("RayTracingPipelineStateNode"),
            nodeitems_utils.NodeItem("RayTracingShaderGroupNode"),
        ],
    ),
]


ray_tracing_shader_group_type_values = (
    ("GENERAL", "General", "", 0),
    ("TRIANGLES_HIT_GROUP", "Triangles Hit Group", "", 1),
    ("PROCEDURAL_HIT_GROUP", "Procedural Hit Group", "", 2),
)

ray_tracing_pipeline_create_flag_bits = (
    ("CREATE_DISABLE_OPTIMIZATION", "Create disable optimization", "", 0x00000001),
    ("CREATE_ALLOW_DERIVATIVES", "Create allow derivatives", "", 0x00000002),
    ("CREATE_DERIVATIVE", "Create derivative", "", 0x00000004),
    (
        "CREATE_VIEW_INDEX_FROM_DEVICE_INDEX",
        "Create view index from device index",
        "",
        0x00000008,
    ),
    ("CREATE_DISPATCH_BASE", "Create dispatch base", "", 0x00000010),
    (
        "RASTERIZATION_STATE_CREATE_FRAGMENT_SHADING_RATE_ATTACHMENT_KHR",
        "Rasterization state create fragment shading rate attachment khr",
        "",
        0x00200000,
    ),
    (
        "RASTERIZATION_STATE_CREATE_FRAGMENT_DENSITY_MAP_ATTACHMENT_EXT",
        "Rasterization state create fragment density map attachment ext",
        "",
        0x00400000,
    ),
    (
        "CREATE_RAY_TRACING_NO_NULL_ANY_HIT_SHADERS_KHR",
        "Create ray tracing no null any hit shaders khr",
        "",
        0x00004000,
    ),
    (
        "CREATE_RAY_TRACING_NO_NULL_CLOSEST_HIT_SHADERS_KHR",
        "Create ray tracing no null closest hit shaders khr",
        "",
        0x00008000,
    ),
    (
        "CREATE_RAY_TRACING_NO_NULL_MISS_SHADERS_KHR",
        "Create ray tracing no null miss shaders khr",
        "",
        0x00010000,
    ),
    (
        "CREATE_RAY_TRACING_NO_NULL_INTERSECTION_SHADERS_KHR",
        "Create ray tracing no null intersection shaders khr",
        "",
        0x00020000,
    ),
    (
        "CREATE_RAY_TRACING_SKIP_TRIANGLES_KHR",
        "Create ray tracing skip triangles khr",
        "",
        0x00001000,
    ),
    (
        "CREATE_RAY_TRACING_SKIP_AABBS_KHR",
        "Create ray tracing skip aabbs khr",
        "",
        0x00002000,
    ),
    (
        "CREATE_RAY_TRACING_SHADER_GROUP_HANDLE_CAPTURE_REPLAY_KHR",
        "Create ray tracing shader group handle capture replay khr",
        "",
        0x00080000,
    ),
    ("CREATE_CAPTURE_STATISTICS_KHR", "Create capture statistics khr", "", 0x00000040),
    (
        "CREATE_CAPTURE_INTERNAL_REPRESENTATIONS_KHR",
        "Create capture internal representations khr",
        "",
        0x00000080,
    ),
    ("CREATE_LIBRARY_KHR", "Create library khr", "", 0x00000800),
)


class RayTracingPipelineInterfaceNodeSocket(bpy.types.NodeSocket):

    bl_label = "Ray Tracing Pipeline Interface node socket"

    def draw(self, context, layout, node, text):
        layout.label(text=text)

    def draw_color(self, context, node):
        return (0.2, 0.9, 0.7, 1.0)


class RayTracingShaderGroupNodeSocket(bpy.types.NodeSocket):

    bl_label = "Ray Tracing Shader Group node socket"

    def draw(self, context, layout, node, text):
        layout.label(text=text)

    def draw_color(self, context, node):
        return (0.9, 0.4, 0.7, 1.0)


class RayTracingPipelineInterfaceNode(bpy.types.Node, RenderTreeNode):

    bl_label = "Ray Tracing Pipeline Interface"

    max_pipeline_ray_payload_size_property: bpy.props.IntProperty(
        name="Max Pipeline Ray Payload Size", default=0, min=0
    )

    max_pipeline_ray_hit_attribute_size_property: bpy.props.IntProperty(
        name="Max Pipeline Ray Hit Attribute Size", default=0, min=0
    )

    def init(self, context):
        self.outputs.new("RayTracingPipelineInterfaceNodeSocket", "Interface")

    def draw_buttons(self, context, layout):
        layout.prop(self, "max_pipeline_ray_payload_size_property")
        layout.prop(self, "max_pipeline_ray_hit_attribute_size_property")


class RayTracingShaderGroupNode(bpy.types.Node, RenderTreeNode):

    bl_label = "Ray Tracing Shader Group"

    type_property: bpy.props.EnumProperty(
        name="Type", items=ray_tracing_shader_group_type_values, default="GENERAL"
    )

    def init(self, context):

        self.inputs.new("PipelineShaderStageNodeSocket", "General Shader")
        self.inputs.new("PipelineShaderStageNodeSocket", "Closest Hit Shader")
        self.inputs.new("PipelineShaderStageNodeSocket", "Any Hit Shader")
        self.inputs.new("PipelineShaderStageNodeSocket", "Intersection Shader")

        self.outputs.new("RayTracingShaderGroupNodeSocket", "Shader Group")

    def draw_buttons(self, context, layout):

        layout.prop(self, "type_property")


class RayTracingPipelineStateNode(bpy.types.Node, RenderTreeNode):

    bl_label = "Ray Tracing Pipeline"

    flags_property: bpy.props.EnumProperty(
        name="Flags",
        items=ray_tracing_pipeline_create_flag_bits,
        options={"ANIMATABLE", "ENUM_FLAG"},
    )
    max_pipeline_ray_recursion_depth_property: bpy.props.IntProperty(
        name="Max Pipeline Ray Recursion Depth Property", default=1
    )
    base_pipeline_index_property: bpy.props.IntProperty(
        name="Base Pipeline Index", default=-1
    )

    def init(self, context):

        self.inputs.new("RayTracingShaderGroupNodeSocket", "Shader Groups")
        self.inputs["Shader Groups"].link_limit = 0

        self.inputs.new("PipelineLibraryNodeSocket", "Library Info")
        self.inputs.new("RayTracingPipelineInterfaceNodeSocket", "Library Interface")
        self.inputs.new("DynamicStateNodeSocket", "Dynamic State")
        self.inputs.new("PipelineLayoutNodeSocket", "Layout")
        self.inputs.new("PipelineNodeSocket", "Base Pipeline")

        self.outputs.new("RayTracingPipelineStateNodeSocket", "Pipeline")

    def draw_buttons(self, context, layout):

        layout.prop(self, "flags_property")
        layout.prop(self, "max_pipeline_ray_recursion_depth_property")
        layout.prop(self, "base_pipeline_index_property")
