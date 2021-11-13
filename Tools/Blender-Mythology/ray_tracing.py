import bpy
import nodeitems_utils
import typing

from .common import find_index, get_input_node
from .pipeline_state import (
    PipelineLayoutNode,
    PipelineNode,
    PipelineShaderStageNode,
    ShaderModuleNode,
    create_pipeline_dynamic_state_json,
    create_pipeline_library_json,
    create_shader_stages_json,
)
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

    name_property: bpy.props.StringProperty(name="Name")
    flags_property: bpy.props.EnumProperty(
        name="Flags",
        items=ray_tracing_pipeline_create_flag_bits,
        options={"ANIMATABLE", "ENUM_FLAG"},
    )
    max_pipeline_ray_recursion_depth_property: bpy.props.IntProperty(
        name="Max Pipeline Ray Recursion Depth Property", default=1
    )

    def init(self, context):

        self.inputs.new("RayTracingShaderGroupNodeSocket", "Shader Groups")
        self.inputs["Shader Groups"].link_limit = 0

        self.inputs.new("PipelineLibraryNodeSocket", "Library Info")
        self.inputs.new("RayTracingPipelineInterfaceNodeSocket", "Library Interface")
        self.inputs.new("PipelineDynamicStateNodeSocket", "Dynamic State")
        self.inputs.new("PipelineLayoutNodeSocket", "Layout")
        self.inputs.new("PipelineNodeSocket", "Base Pipeline")

        self.outputs.new("PipelineNodeSocket", "Pipeline")

    def draw_buttons(self, context, layout):
        layout.prop(self, "name_property")
        layout.prop(self, "flags_property")
        layout.prop(self, "max_pipeline_ray_recursion_depth_property")


JSONType = typing.Union[
    str, int, float, bool, None, typing.Dict[str, typing.Any], typing.List[typing.Any]
]


def gather_ray_tracing_shader_stages(
    shader_group_nodes: typing.List[RayTracingShaderGroupNode],
) -> typing.List[PipelineShaderStageNode]:

    shader_stages_per_group = [
        [
            input.links[0].from_node if (len(input.links) == 1) else None
            for input in shader_group.inputs
            if input.name == "General Shader"
            or input.name == "Closest Hit Shader"
            or input.name == "Any Hit Shader"
            or input.name == "Intersection Shader"
        ]
        for shader_group in shader_group_nodes
    ]

    shader_stages = [
        shader_stage
        for shader_stages in shader_stages_per_group
        for shader_stage in shader_stages
        if shader_stage != None
    ]

    return shader_stages


def create_ray_tracing_shader_group_json(
    shader_group_node: RayTracingShaderGroupNode,
    pipeline_shader_stage_nodes: typing.List[PipelineShaderStageNode],
) -> JSONType:

    json = {
        "type": shader_group_node.get("type_property", 0),
    }

    if len(shader_group_node.inputs["General Shader"].links) == 1:
        json["general_shader"] = find_index(
            pipeline_shader_stage_nodes,
            get_input_node(shader_group_node, "General Shader", 0),
        )

    if len(shader_group_node.inputs["Closest Hit Shader"].links) == 1:
        json["closest_hit_shader"] = find_index(
            pipeline_shader_stage_nodes,
            get_input_node(shader_group_node, "Closest Hit Shader", 0),
        )

    if len(shader_group_node.inputs["Any Hit Shader"].links) == 1:
        json["any_hit_shader"] = find_index(
            pipeline_shader_stage_nodes,
            get_input_node(shader_group_node, "Any Hit Shader", 0),
        )

    if len(shader_group_node.inputs["Intersection Shader"].links) == 1:
        json["intersection_shader"] = find_index(
            pipeline_shader_stage_nodes,
            get_input_node(shader_group_node, "Intersection Shader", 0),
        )

    return json


def create_ray_tracing_pipeline_interface_json(
    node: RayTracingPipelineInterfaceNode,
) -> JSONType:

    return {
        "max_pipeline_ray_payload_size": node.get(
            "max_pipeline_ray_payload_size_property", 0
        ),
        "max_pipeline_ray_hit_attribute_size": node.get(
            "max_pipeline_ray_hit_attribute_size_property", 0
        ),
    }


def ray_tracing_pipeline_state_to_json(
    pipeline_node: RayTracingPipelineStateNode,
    shader_modules: typing.Tuple[typing.List[ShaderModuleNode], JSONType],
    pipeline_layouts: typing.Tuple[typing.List[PipelineLayoutNode], JSONType],
    pipelines: typing.List[PipelineNode],
) -> JSONType:

    pipeline_shader_stages = gather_ray_tracing_shader_stages(
        [
            shader_group_link.from_node
            for shader_group_link in pipeline_node.inputs["Shader Groups"].links
        ]
    )

    json = {
        "name": pipeline_node.name_property,
        "flags": pipeline_node.get("stage_flags_property", 0),
        "stages": create_shader_stages_json(pipeline_shader_stages, shader_modules),
        "groups": [
            create_ray_tracing_shader_group_json(
                group_link.from_node, pipeline_shader_stages
            )
            for group_link in pipeline_node.inputs["Shader Groups"].links
        ],
        "max_pipeline_ray_recursion_depth": pipeline_node.get(
            "max_pipeline_ray_recursion_depth_property", 0
        ),
        "library_info": create_pipeline_library_json(
            pipeline_node.inputs["Library Info"].links[0].from_node
        ),
        "library_interface": create_ray_tracing_pipeline_interface_json(
            pipeline_node.inputs["Library Interface"].links[0].from_node
        ),
        "dynamic_state": create_pipeline_dynamic_state_json(
            pipeline_node.inputs["Dynamic State"]
        ),
        "layout": find_index(
            pipeline_layouts, pipeline_node.inputs["Layout"].links[0].from_node
        ),
    }

    if len(pipeline_node.inputs["Base Pipeline"].links) == 1:
        json["base_pipeline"] = find_index(
            pipelines, pipeline_node.inputs["Base Pipeline"].links[0].from_node
        )

    return json
