import bpy
import nodeitems_utils

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


class RayTracingPipelineStateNodeSocket(bpy.types.NodeSocket):

    bl_label = "Ray Tracing Pipeline State node socket"

    def draw(self, context, layout, node, text):
        layout.label(text=text)

    def draw_color(self, context, node):
        return (0.4, 0.7, 0.9, 1.0)


class RayTracingShaderGroupNodeSocket(bpy.types.NodeSocket):

    bl_label = "Ray Tracing Shader Group node socket"

    def draw(self, context, layout, node, text):
        layout.label(text=text)

    def draw_color(self, context, node):
        return (0.9, 0.4, 0.7, 1.0)


class RayTracingShaderGroupNode(bpy.types.Node, RenderTreeNode):

    bl_label = "Ray Tracing Shader Group"

    type_property: bpy.props.EnumProperty(
        name="Type", items=ray_tracing_shader_group_type_values, default="GENERAL"
    )
    # TODO

    def init(self, context):

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
    # TODO

    def init(self, context):

        # TODO maybe this will be in the ShaderGroup
        self.inputs.new("PipelineShaderStageNodeSocket", "Stages")
        self.inputs["Stages"].link_limit = 0

        self.outputs.new("RayTracingPipelineStateNodeSocket", "Pipeline")

    def draw_buttons(self, context, layout):

        layout.prop(self, "flags_property")
